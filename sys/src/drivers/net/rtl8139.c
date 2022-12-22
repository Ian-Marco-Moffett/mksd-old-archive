/*
 *  Description: RTL8139 NIC driver.
 *  Author(s): Ian Marco Moffett.
 *
 */


#include <drivers/net/rtl8139.h>
#include <arch/bus/pci.h>
#include <arch/x64/io.h>
#include <arch/x64/idt.h>
#include <arch/x64/lapic.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <mm/pmm.h>
#include <lib/log.h>
#include <lib/asm.h>
#include <lib/string.h>
#include <lib/math.h>
#include <net/ip.h>
#include <net/icmp.h>
#include <net/arp.h>
#include <intr/irq.h>

#define RTL8139_DEBUG 1

/* Packets */
#define PACKET_SIZE_MAX 0x600
#define PACKET_SIZE_MIN 0x16

/* Buffers */
#define RX_BUFFER_SIZE 32768
#define TX_BUFFER_SIZE PACKET_SIZE_MAX
#define TX_BUFFER_COUNT 4

/* Registers */
#define REG_MAC       0x00
#define REG_MAR0      0x08
#define REG_MAR4      0x12
#define REG_TXSTATUS0 0x10
#define REG_TXADDR0   0x20
#define REG_RXBUF     0x30
#define REG_COMMAND   0x37
#define REG_CAPR      0x38
#define REG_IMR       0x3C
#define REG_ISR       0x3E
#define REG_TXCFG     0x40
#define REG_RXCFG     0x44
#define REG_MPC       0x4C
#define REG_CFG9346   0x50
#define REG_CONFIG1   0x52
#define REG_MSR       0x58
#define REG_BMCR      0x62
#define REG_ANLPAR    0x68

/* Commands */
#define CMD_RX_EMPTY  0x01
#define CMD_TX_ENABLE 0x04
#define CMD_RX_ENABLE 0x08
#define CMD_RESET     0x10

/* Register config */
#define CFG9346_NONE  0x00
#define CFG9346_EEM0  0x40
#define CFG9346_EEM1  0x80

/* Basic mode control register */
#define BMCR_SPEED          0x2000
#define BMCR_AUTO_NEGOTIATE 0x1000
#define BMCR_DUPLEX         0x0100

#define MSR_LINKB 0x02
#define MSR_SPEED_10 0x08
#define MSR_RX_FLOW_CONTROL_ENABLE 0x40

/* Recieve configuration */
#define RXCFG_AAP               0x01
#define RXCFG_APM               0x02
#define RXCFG_AM                0x04
#define RXCFG_AB                0x08
#define RXCFG_AR                0x10
#define RXCFG_WRAP_INHIBIT      0x80
#define RXCFG_MAX_DMA_16B       0x000
#define RXCFG_MAX_DMA_32B       0x100
#define RXCFG_MAX_DMA_64B       0x200
#define RXCFG_MAX_DMA_128B      0x300
#define RXCFG_MAX_DMA_256B      0x400
#define RXCFG_MAX_DMA_512B      0x500
#define RXCFG_MAX_DMA_1K        0x600
#define RXCFG_MAX_DMA_UNLIMITED 0x0700
#define RXCFG_RBLN_8K           0x0000
#define RXCFG_RBLN_16K          0x0800
#define RXCFG_RBLN_32K          0x1000
#define RXCFG_RBLN_64K          0x1800
#define RXCFG_FTH_NONE          0xE000

/* Transmit configuration */
#define TXCFG_TXRR_ZERO         0x00
#define TXCFG_MAX_DMA_16B       0x000
#define TXCFG_MAX_DMA_32B       0x100
#define TXCFG_MAX_DMA_64B       0x200
#define TXCFG_MAX_DMA_128B      0x300
#define TXCFG_MAX_DMA_256B      0x400
#define TXCFG_MAX_DMA_512B      0x500
#define TXCFG_MAX_DMA_1K        0x600
#define TXCFG_MAX_DMA_2K        0x700
#define TXCFG_IFG11             0x3000000

/* Interrupts */
#define INT_RXOK               0x01
#define INT_RXERR              0x02
#define INT_TXOK               0x04
#define INT_TXERR              0x08
#define INT_RX_BUFFER_OVERFLOW 0x10
#define INT_LINK_CHANGE        0x20
#define INT_RX_FIFO_OVERFLOW   0x40
#define INT_LENGTH_CHANGE      0x2000
#define INT_SYSTEM_ERROR       0x8000

/* Transfer status */
#define TX_STATUS_OWN           0x2000
#define TX_STATUS_THRESHOLD_MAX 0x3F0000

/* Recieve status */
#define RX_MULTICAST             0x8000
#define RX_PHYSICAL_MATCH        0x4000
#define RX_BROADCAST             0x2000
#define RX_INVALID_SYMBOL_ERROR  0x0020
#define RX_RUNT                  0x0010
#define RX_LONG                  0x0008
#define RX_CRC_ERROR             0x0004
#define RX_FRAME_ALIGNMENT_ERROR 0x0002
#define RX_OK                    0x0001

#define VENDOR_ID 0x10EC
#define DEVICE_ID 0x8139

static size_t next_txbuf = 0;
static uint8_t txbufs[TX_BUFFER_COUNT];
static pci_device_t* dev = NULL;
static uint32_t iobase = 0;
static void* rxbuf = NULL;
static uint8_t got_packet = 0;
static off_t rxbuf_off = 0;
static void* packet_buf = NULL;

mac_address_t g_rtl8139_mac_addr;

static void
arp_respond(arp_packet_t* arp_packet)
{
  if (RTL8139_DEBUG)
  {
    printk(PRINTK_INFO "Got ARP request, replying..\n");
  }

  arp_packet_t* packet = kmalloc(sizeof(arp_packet_t));
  packet->htype = BIG_ENDIAN(ARP_HW_ETHERNET);
  
  /* XXX: Possibly update this */
  packet->ptype = BIG_ENDIAN(ETHERTYPE_IPV4);
  packet->haddr_len = sizeof(mac_address_t);
  packet->paddr_len = sizeof(ipv4_address_t);
  packet->op = BIG_ENDIAN(ARP_OP_REPLY);
  packet->target_paddr = arp_packet->sender_paddr;
  
  /* -----------------*/
  memcpy(packet->target_haddr, arp_packet->sender_haddr,
         sizeof(mac_address_t));

  packet->sender_paddr = arp_packet->target_paddr;
  memcpy(packet->sender_haddr, g_rtl8139_mac_addr, sizeof(mac_address_t));
  ethernet_send(packet->target_haddr, ETHERTYPE_ARP,
                (uint8_t*)packet, sizeof(arp_packet_t));
}

/* 
 * Identifies a packet and responds 
 * if needed
 */
static void
id_packet(void)
{
  mask_irq(dev->irq_line);
  ethernet_header_t* hdr = (ethernet_header_t*)packet_buf;
  
  switch (hdr->ether_type)
  {
    case BIG_ENDIAN(ETHERTYPE_ARP):
      {
        arp_packet_t* arp_packet =
          (arp_packet_t*)((uint64_t)hdr + sizeof(ethernet_header_t));

        arp_respond(arp_packet);
      }
      break;
  }
  
  unmask_irq(dev->irq_line);
}

static uint8_t
is_link_up(void) {
  return ((inb(iobase + REG_MSR) & MSR_LINKB) == 0);
}

static uint8_t 
get_speed_mbps(void) {
  uint16_t msr = inw(iobase + REG_MSR);
  return msr & MSR_SPEED_10 ? 10 : 100;
}

static void
update_mac_addr(void) {
  for (unsigned int i = 0; i < 6; ++i) {
    g_rtl8139_mac_addr[i] = inb(iobase + REG_MAC + i);
  }
}

static void
recieve_packet(void)
{
  uint8_t* packet = (uint8_t*)((uint64_t)rxbuf + rxbuf_off);
  uint16_t status = *(uint16_t*)(packet + 0);
  uint16_t length = *(uint16_t*)(packet + 2);

  const uint8_t IS_BAD_PACKET =
                !(status & RX_OK) 
                || (status & (RX_INVALID_SYMBOL_ERROR 
                              | RX_CRC_ERROR 
                              | RX_FRAME_ALIGNMENT_ERROR)) 
                || (length >= PACKET_SIZE_MAX) 
                || (length < PACKET_SIZE_MIN);

  if (IS_BAD_PACKET && RTL8139_DEBUG)
  {
    printk(PRINTK_WARN "RTL8139: Got bad packet (status=%x, length=%x)\n", 
           status, length);

    if (status & RX_INVALID_SYMBOL_ERROR)
    {
      printk(PRINTK_WARN "RTL8139: Invalid symbol.\n");
    }

    if (status & RX_CRC_ERROR)
    {
      printk(PRINTK_WARN "RTL8139: CRC error.\n");
    }

    if (status & RX_FRAME_ALIGNMENT_ERROR)
    {
      printk(PRINTK_WARN "RTL8139: Frame alignment error.\n");
    }

    if (length >= PACKET_SIZE_MAX)
    {
      printk(PRINTK_WARN "RTL8139: Packet size too long!\n");
    }

    if (length < PACKET_SIZE_MIN)
    {
      printk(PRINTK_WARN "RTL8139: Packet size too small!\n");
    }

    return;
  }
  
  memcpy(packet_buf, (uint8_t*)((uintptr_t)packet + 4), length - 4);
  rxbuf_off = ((rxbuf_off + length + 4 + 3) & ~3) % RX_BUFFER_SIZE;
  outw(iobase + REG_CAPR, rxbuf_off - 0x10);
  rxbuf_off %= RX_BUFFER_SIZE;

  got_packet = 1;
  if (RTL8139_DEBUG)
  {
    printk(PRINTK_INFO "RTL8139(DEBUG): Recieved %d bytes of data.\n", length);
  }

  id_packet();
}

_isr static void
rtl8139_isr(void* stackframe)
{
  
  mask_irq(dev->irq_line);
  while (1)
  {
    uint16_t status = inw(iobase + REG_ISR);
    outw(iobase + REG_ISR, status);
    const size_t STAT = INT_RXOK 
                       | INT_RXERR 
                       | INT_TXOK 
                       | INT_TXERR 
                       | INT_RX_BUFFER_OVERFLOW 
                       | INT_LINK_CHANGE 
                       | INT_RX_FIFO_OVERFLOW 
                       | INT_LENGTH_CHANGE 
                       | INT_SYSTEM_ERROR;

    if (!(status & STAT))
    {
      break;
    }

    if ((status & INT_TXOK) && RTL8139_DEBUG)
    {
      printk(PRINTK_INFO "RTL8139: TX complete.\n");
    }

    if ((status & INT_TXERR) && RTL8139_DEBUG)
    {
      printk(PRINTK_WARN "RTL8139: TX error.\n");
    }

    if (status & INT_RXOK && RTL8139_DEBUG)
    {
      printk(PRINTK_INFO "RTL8139(DEBUG): Recieved packet!\n");
      recieve_packet();
    }

    if (status & INT_RXERR && RTL8139_DEBUG)
    {
      printk(PRINTK_WARN "RTL8139: RX error.\n");
    }

    if (status & INT_RX_BUFFER_OVERFLOW && RTL8139_DEBUG)
    {
      printk(PRINTK_WARN "RTL8139: RX buffer overflow.\n");
    }

    if (status & INT_LINK_CHANGE && RTL8139_DEBUG)
    {
      printk(PRINTK_INFO "RTL8139(DEBUG): Link status has changed, "
                         "STATE=%s\n", is_link_up() ? "UP" : "DOWN");
    }
  }

  outw(iobase + REG_ISR, 0x5);
  lapic_send_eoi();
  unmask_irq(dev->irq_line);
}


void 
rtl8139_send_packet(void* data, size_t size)
{
  if (iobase == 0)
  {
    return;
  }

  ssize_t hwbuf = -1;

  for (unsigned int i = 0; i < TX_BUFFER_COUNT; ++i)
  {
    size_t canidate = (next_txbuf + i) % 4;
    uint32_t status = inl(iobase + REG_TXSTATUS0 + (canidate * 4));

    if (status & TX_STATUS_OWN)
    {
      hwbuf = canidate;
      break;
    }
  }

  next_txbuf = (hwbuf + 1) % 4;
  if (size < 60) size = 60;

  uintptr_t phys_addr = txbufs[hwbuf];
  uintptr_t virt_addr = phys_addr + VMM_HIGHER_HALF;
  memzero((void*)virt_addr, TX_BUFFER_SIZE - size);
  memcpy((void*)virt_addr, data, size);
  outl(iobase + REG_TXSTATUS0 + (hwbuf * 4), size);
}


uint8_t
rtl8139_got_packet(void)
{
  if (!(got_packet))
  {
    return 0;
  }

  return 1;
}


uint8_t*
rtl8139_read_packet(void)
{
  got_packet = 0;
  return packet_buf;
}

void 
rtl8139_init(void)
{
  dev = pci_find(VENDOR_ID, DEVICE_ID);

  if (dev == NULL)
  {
    return;
  }

  printk(PRINTK_INFO "RTL8139: RTL8139 found on PCI bus %d, slot %d\n",
         dev->bus, dev->slot);

  /* Enable bus mastering so the NIC can perform DMA */
  pci_enable_bus_mastering(dev);
  iobase = dev->bars[0] & 0xFFFFFFFC;

  printk(PRINTK_INFO "RTL8139: Resetting NIC..\n");

  outb(iobase + REG_COMMAND, CMD_RESET);
  while (inb(iobase + REG_COMMAND) & CMD_RESET);

  /* Put the NIC into config write enable mode */
  outb(iobase + REG_CFG9346, CFG9346_EEM1 | CFG9346_EEM0);

  /* Enable multicast */
  outl(iobase + REG_MAR0, 0xFFFFFFFF);
  outl(iobase + REG_MAR4, 0xFFFFFFFF);
  printk(PRINTK_INFO "RTL8139: Multicast enabled.\n");

  /* Enable RX and TX */
  outb(iobase + REG_COMMAND, CMD_RX_ENABLE | CMD_TX_ENABLE);
  printk(PRINTK_INFO "RTL8139: RX and TX enabled.\n");

  /* Turn on the NIC */
  outl(iobase + REG_CONFIG1, 0);

  /* Allocate memory for the RX buffer */
  rxbuf = (void*)((uintptr_t)pmm_alloc(RX_BUFFER_SIZE/4096) + VMM_HIGHER_HALF);

  /* 
   * Set the RX buffer to the physical address
   * of the memory allocated
   */
  outl(iobase + REG_RXBUF, (uintptr_t)rxbuf - VMM_HIGHER_HALF);
  
  /* 
   * Basic mode control configuration,
   * 100mbit full duplex auto negoiation mode
   */
  outl(iobase + REG_BMCR, BMCR_SPEED  | BMCR_AUTO_NEGOTIATE | BMCR_DUPLEX);

  /* Enable control flow */
  outb(iobase + REG_MSR, MSR_RX_FLOW_CONTROL_ENABLE);

  /* Set RX rules: accept rtl8139 MAC match, multicast, 
   * and broadcasted packets.
   * Also use max DMA transfer size and no FIFO threshold.
   */
  outl(iobase + REG_RXCFG, RXCFG_APM 
                           | RXCFG_AM 
                           | RXCFG_AB 
                           | RXCFG_WRAP_INHIBIT 
                           | RXCFG_MAX_DMA_UNLIMITED 
                           | RXCFG_RBLN_32K 
                           | RXCFG_FTH_NONE);

  /*  
   *  Set TX mode to use default retry count, 
   *  max DMA burst size and interframe
   *  gap time 
   */
  outl(iobase + REG_TXCFG, TXCFG_TXRR_ZERO 
                           | TXCFG_MAX_DMA_1K 
                           | TXCFG_IFG11);

  for (unsigned int i = 0; i < TX_BUFFER_COUNT; ++i)
  {
    txbufs[i] = pmm_alloc(1);
  }

  outb(iobase + REG_CFG9346, CFG9346_NONE);

  outw(iobase + REG_IMR, INT_RXOK
                         | INT_RXERR
                         | INT_TXOK
                         | INT_TXERR
                         | INT_RX_BUFFER_OVERFLOW
                         | INT_LINK_CHANGE
                         | INT_RX_FIFO_OVERFLOW
                         | INT_LENGTH_CHANGE
                         | INT_SYSTEM_ERROR);
  outw(iobase + REG_ISR, 0xFFFF);

  if (is_link_up())
  {
    printk(PRINTK_INFO "RTL8139: Link up @%dmbps!\n", get_speed_mbps());
  }
  else
  {
    printk(PRINTK_WARN "RTL8139: Link down.\n");
  }

  update_mac_addr();
  printk(PRINTK_INFO "RTL8139: MAC address: %X:%X:%X:%X:%X:%X\n",
      g_rtl8139_mac_addr[0], 
      g_rtl8139_mac_addr[1], 
      g_rtl8139_mac_addr[2], 
      g_rtl8139_mac_addr[3], 
      g_rtl8139_mac_addr[4], 
      g_rtl8139_mac_addr[5]);

  packet_buf = kmalloc(PACKET_SIZE_MAX);
  register_irq(dev->irq_line, rtl8139_isr);
  asmv("sti");
}
