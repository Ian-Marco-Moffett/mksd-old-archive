/*
 *  Description: AHCI SATA driver.
 *  Author(s): Ian Marco Moffett
 *
 */



#include <drivers/hdd/ahci.h>
#include <arch/bus/pci.h>
#include <lib/log.h>
#include <lib/asm.h>
#include <lib/string.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <mm/heap.h>
#include <fs/devfs.h>

#define CLASS_ID    0x1
#define SUBCLASS_ID 0x6

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define SATA_SIG_ATA 0x00000101     /* SATA drive */
#define SATA_SIG_ATAPI 0xEB140101   /* SATAPI drive */
#define SATA_SIG_SEMB 0xC33C0101    /* Enclosure management bridge */
#define SATA_SIG_PM 0x96690101      /* Port multiplier */

#define HBA_PxCMD_ST    (1 << 0)
#define HBA_PxCMD_FRE   (1 << 4)
#define HBA_PxCMD_FR    (1 << 14)
#define HBA_PxCMD_CR    (1 << 15)

#define HBA_PxIS_TFES      (1 << 30)
#define HBA_PxIS_HBFS      (1 << 29)
#define HBA_PxIS_IFS       (1 << 27)
#define HBA_PxIS_HBDS      (1 << 28)
#define HBA_PxIS_INFS      (1 << 26)

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4

static pci_device_t* dev = NULL;
static size_t n_hba_ports = 0;
static size_t n_cmdslots = 0;
static volatile HBA_MEM* abar = NULL;
static volatile HBA_PORT* main_drive = NULL;


static void
open(vfs_node_t* node)
{
  printk("/dev/sda has been opened!\n");
}

static fops_t fops = {
  .open = open
};


static int 
find_cmdslot(HBA_PORT* port)
{
  uint32_t slots = (port->sact | port->ci);
  for (int i = 0; i < n_cmdslots; ++i)
  {
    if (!(slots & (1 << i)))
    {
      return i;
    }
  }

  return -1;
}


/*
 * This function causes
 * the HBA to stop posting
 * recieved FISes into the
 * FIS receive area AND
 * prevents the HBA 
 * from getting in commands.
 *
 * This is to prevent 
 * funky things from
 * happening.
 *
 */

static void
stop_cmd_engine(HBA_PORT* port)
{
  // Unset FRE.
  port->cmd &= ~(1 << 4);
  while (port->cmd & (1 << 14));
  
  // Unset ST.
  port->cmd &= ~(1 << 0);
  while (port->cmd & (1 << 15));
}


/*
 *  This function starts up 
 *  the command engine
 *  so the HBA can receive
 *  FISes and process commands.
 *
 */

static void
start_cmd_engine(HBA_PORT* port)
{
  // Set ST.
  port->cmd |= (1 << 0);
  while (!(port->cmd & (1 << 15)));

  // Set FRE.
  port->cmd |= (1 << 4);
  while (!(port->cmd & (1 << 14)));
}


static void 
send_cmd(HBA_PORT* port, uint32_t slot) 
{
  while ((port->tfd & 0x88) != 0);
  port->cmd &= ~(1 << 0);
  while ((port->cmd & HBA_PxCMD_CR) != 0);

  port->cmd |= HBA_PxCMD_FR | HBA_PxCMD_ST;
  port->ci = 1 << slot;

  while (port->ci & (1 << slot) != 0);

  port->cmd &= ~(HBA_PxCMD_ST);
  while ((port->cmd & HBA_PxCMD_ST) != 0);
  port->cmd &= ~(HBA_PxCMD_FRE);
}

/*
 *  This function will first check
 *  if the HBA supports 64-bit addressing
 *  as it is needed as of now (XXX: UPDATE IF NEEDED).
 *
 *  If 64-bit addressing is not supported
 *  then a warning will be printed out
 *  and the setup will be aborted.
 *
 *  We will also keep track
 *  of how many ports are
 *  supported on the HBA.
 *
 *  @returns: 0 if success, 1 if failure.
 *
 *
 */

static uint8_t
read_hba_cap(void)
{
  printk(PRINTK_INFO "AHCI: Checking if HBA supports 64 " 
                     "bit addressing..\n");
  if (!(abar->cap & (1 << 31)))
  {
    printk(PRINTK_WARN "AHCI: HBA does not support 64 bit addressing "
                       "(aborting HBA setup)\n");

    return 1;
  }

  printk(PRINTK_INFO "AHCI: HBA supports 64 bit addressing!\n");
  
  /*
   *  AHCI spec states the amount of ports
   *  the HBA silicon supports is advertised
   *  in bits 4:0 of the HBA capabilities register.
   */
  n_hba_ports = abar->cap & 0xF;
  printk(PRINTK_INFO "AHCI: HBA silicon supports %d %s.\n",
         n_hba_ports+1, n_hba_ports+1 > 1 ? "ports" : "port");

  /* Fetch the amount of command slots in bits 12:8*/
  n_cmdslots = (abar->cap >> 8) & 0x1F;
  printk(PRINTK_INFO "AHCI: HBA supports %d command %s.\n",
         n_cmdslots, n_cmdslots > 1 ? "slots" : "slot");

  return 0;
}

/*
 *  Performs a BIOS handoff
 *  to give control of the HBA
 *  from the BIOS to us.
 *
 *  This is an important step!
 *
 *  @returns: 0 if success, 1 if failure.
 *
 *
 */

static uint8_t 
take_ownership(void) 
{
  if (!(abar->cap & (1 << 0))) 
  {
    printk(PRINTK_WARN "AHCI: BIOS handoff not supported!\n");
    return 1;
  }

  abar->bohc |= (1 << 1);  
  while (abar->bohc & (1 << 0) == 0);

  for (uint32_t i = 0; i < 5; ++i)
  {
    CLI_SLEEP;
  }

  if (abar->bohc & (1 << 4)) 
  {
    for (uint32_t i = 0; i < 10; ++i)
    {
      CLI_SLEEP;
    }
  }

  if (abar->bohc & (1 << 4) != 0 || 
      abar->bohc & (1 << 0) != 0 || 
      abar->bohc & (1 << 1) == 0)
  {
    printk(PRINTK_WARN "AHCI: BIOS handoff failure!\n");
    return 1;
  }

  printk(PRINTK_INFO "AHCI: BIOS handoff success.\n");
  return 0;
}

/*
 *  This function returns
 *  the type of drive
 *  attached to a port.
 *
 */

static int
get_drive_type(HBA_PORT* port)
{
  uint32_t ssts = port->ssts;
  uint8_t ipm = (ssts >> 8) & 0xF;
  uint8_t det = ssts & 0xF;

  if (det != HBA_PORT_DET_PRESENT || ipm != HBA_PORT_IPM_ACTIVE)
  {
    return AHCI_DEV_NULL;
  }

  switch (port->sig)
  {
    case SATA_SIG_ATAPI:
      return AHCI_DEV_SATAPI;
    case SATA_SIG_SEMB:
      return AHCI_DEV_SEMB;
    case SATA_SIG_PM:
      return AHCI_DEV_PM;
    default:
      return AHCI_DEV_SATA;
  }
}


static void
swap_endianess(char* buf, size_t n)
{
  for (size_t i = 0; i < n; i += 2)
  {
    uint8_t tmp = buf[i];
    buf[i] = buf[i + 1];
    buf[i + 1] = tmp;
  }
}


static void 
print_char_arr(char* buf, size_t n)
{
  for (size_t i = 0; i < n; ++i)
  {
    if (buf[i] == ' ')
    {
      continue;
    }

    printk("%c", buf[i]);
  }

  printk("\n");
}


/*
 *  Prints out some information
 *  during initialization.
 *
 */

static void 
get_port_info(HBA_PORT* port)
{
  printk(PRINTK_INFO "AHCI: Port hot plug capable: %s\n",
         port->cmd & (1 << 18) ? "yes" : "no");

  printk(PRINTK_INFO "AHCI: Mechanical presence switch on port: %s\n",
         port->cmd & (1 << 9) ? "yes" : "no");

  /* Fetch a commandslot */
  int cmdslot = find_cmdslot(port);
  if (cmdslot == -1)
  {
    printk(PRINTK_WARN "Something went wrong "
                       "(no free cmdslot)\n");
    return;
  }

  /* Build a command header*/
  uint64_t clb = ((uint64_t)port->clbu << 32 | port->clb);
  HBA_CMD_HEADER* cmdhdr = (HBA_CMD_HEADER*)(clb + VMM_HIGHER_HALF);
  cmdhdr[cmdslot].prdtl = 1;
  cmdhdr[cmdslot].cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);
  cmdhdr[cmdslot].w = 0;
  cmdhdr[cmdslot].p = 0;
  
  /* Get command table base */
  uint64_t ctba = ((uint64_t)cmdhdr[cmdslot].ctbau << 32 
                             | cmdhdr[cmdslot].ctba);
  
  /* Set the command table and null it */
  HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)(ctba + VMM_HIGHER_HALF);
  memzero(cmdtbl, sizeof(HBA_CMD_TBL));

  
  /* 
   * Allocate some physical memory
   * for use as an identity buffer.
   */
  uintptr_t buf = pmm_alloc(1);
  memzero((void*)(buf + VMM_HIGHER_HALF), 4096);

  /* Set up the cmdtbl entries */
  cmdtbl->prdt[0].dba = (uint32_t)buf;
  cmdtbl->prdt[0].dbau = (uint32_t)(buf >> 32);
  cmdtbl->prdt[0].dbc = 511;

  FIS_REG_H2D* cmd = (FIS_REG_H2D*)cmdtbl->cfis;
  cmd->command = 0xEC;
  cmd->c = 1;
  cmd->fis_type = FIS_TYPE_REG_H2D;
  send_cmd(port, cmdslot);

  char* serial_num = kmalloc(21);
  char* model_num = kmalloc(41);

  memcpy(serial_num, (uint8_t*)(buf + 20), 20); 
  swap_endianess(serial_num, 20);

  memcpy(model_num, (uint8_t*)(buf + 54), 40);
  swap_endianess(model_num, 40);

  /* Write out the serial number */
  printk(PRINTK_INFO "AHCI: Drive serial number: ");
  print_char_arr(serial_num, 21);

  /* Write out model number */
  printk(PRINTK_INFO "AHCI: Drive model number: "); 
  print_char_arr(model_num, 21);

  kfree(serial_num);
  kfree(model_num);
}



/*
 *  This function sets
 *  up a device on an
 *  HBA port.
 *
 *  NOTE: Should only be called once
 *        per port.
 *
 *
 */

static void
init_port_single(HBA_PORT* port)
{
  /* Stop the command engine */
  printk(PRINTK_INFO "AHCI: Waiting for HBA command engine to stop..\n");
  stop_cmd_engine(port);
  printk(PRINTK_INFO "AHCI: Successfully stopped HBA command engine!\n");
  printk(PRINTK_INFO "AHCI: Initializing HBA port..\n");
  
  /* Set up the command list */
  uintptr_t clb = pmm_alloc(1);
  port->clb = (uint32_t)clb;
  port->clbu = (uint32_t)(clb >> 32);

  /* Set up FIS buffer */
  uint64_t fb = pmm_alloc(1);
  port->fb = (uint32_t)fb;
  port->fbu = (uint32_t)(fb >> 32);

  HBA_CMD_HEADER* cmdhdr = (HBA_CMD_HEADER*)(port->clb + VMM_HIGHER_HALF);
  for (size_t i = 0; i < 32; ++i)
  {
    uintptr_t desc_base = pmm_alloc(1);
    cmdhdr[i].ctba = (uint32_t)desc_base;
    cmdhdr[i].ctbau = (uint32_t)(desc_base >> 32);
  }

  cmdhdr->prdtl = 1;
  
  /* Start up the command engine */
  printk(PRINTK_INFO "AHCI: Starting up HBA command engine..\n");
  start_cmd_engine(port);

  /* Dump some information */
  get_port_info(port);
}


/*
 *  This function sets up all ports
 *  on the HBA.
 *
 *  NOTE: This as of now only sets up 
 *        SATA drives.
 *
 *
 */

static void
init_ports(void)
{
  uint32_t pi = abar->pi;
  size_t drive_count = 0;
  char* drive_id = kmalloc(sizeof(char) * (23));

  for (uint32_t i = 0; i < 32; ++i)
  {
    if (pi & (1 << i))
    {
      int dt = get_drive_type(&abar->ports[i]);
      switch (dt)
      {
        case AHCI_DEV_SATA:
          printk(PRINTK_INFO "AHCI: SATA drive found @HBA_PORT_%d\n", i);
          init_port_single(&abar->ports[i]);
          printk(PRINTK_INFO "AHCI: Port %d initialized "
                             "successfully!\n", i); 

          if (main_drive == NULL)
          {
            main_drive = &abar->ports[i];
          }

          if (drive_count == 0)
          {
            devfs_register_device("sda", &fops);
          }
          else
          {
            snprintf(drive_id, 23, "sda%d", drive_count);
            devfs_register_device(drive_id, &fops);
          }

          
          ++drive_count;
          memzero(drive_id, 23);
          break;
      }
    }
  }

  kfree(drive_id);
}


int
ahci_write_drive(uint64_t lba, uint16_t* buf_phys, uint8_t n_sectors)
{
  int cmdslot = find_cmdslot(main_drive);

  if (cmdslot == -1)
  {
    return 1;
  }

  /* Build a command header */
  uint64_t clb = ((uint64_t)main_drive->clbu << 32 | main_drive->clb);
  HBA_CMD_HEADER* cmdhdr = (HBA_CMD_HEADER*)(clb + VMM_HIGHER_HALF);
  cmdhdr[cmdslot].prdtl = 1;
  cmdhdr[cmdslot].cfl = sizeof(FIS_REG_H2D)/sizeof(uint32_t);
  cmdhdr[cmdslot].w = 1;
  cmdhdr[cmdslot].p = 0;

  /* Get the command table base and null it */
  uint64_t ctba = ((uint64_t)cmdhdr[cmdslot].ctbau << 32
                             | cmdhdr[cmdslot].ctba);
  HBA_CMD_TBL* cmdtbl = (HBA_CMD_TBL*)(ctba + VMM_HIGHER_HALF);
  memzero(cmdtbl, sizeof(HBA_CMD_TBL));

  uintptr_t buf = (uintptr_t)buf_phys;

  /* Set up cmdtbl prdt entries */
  cmdtbl->prdt[0].dba = (uint32_t)buf;
  cmdtbl->prdt[0].dbau = (uint32_t)(buf >> 32);
  cmdtbl->prdt[0].dbc = (n_sectors*512)-1;
  
  FIS_REG_H2D* cmd = (FIS_REG_H2D*)cmdtbl->cfis;
  cmd->fis_type = FIS_TYPE_REG_H2D;
  cmd->command = 0x35;        /* Write DMA extended */
  cmd->c = 1;
  cmd->lba0   = (uint8_t)lba;
  cmd->lba1   = (uint8_t)(lba >> 8);
  cmd->lba2   = (uint8_t)(lba >> 16);
  cmd->device = 64;
  cmd->lba3   = (uint8_t)(lba >> 24);
  cmd->lba4   = (uint8_t)(lba >> 32);
  cmd->lba5   = (uint8_t)(lba >> 40);
  cmd->countl = (uint8_t)n_sectors;
  cmd->counth = (uint8_t)(n_sectors >> 8);
  send_cmd(main_drive, cmdslot);

  return 0;
}


void
ahci_init(void)
{
  dev = pci_find_any(CLASS_ID, SUBCLASS_ID, -1);
  if (dev == NULL)
  {
    return;
  }
  
  /* Fetch BAR5 (ABAR) */
  abar = (HBA_MEM*)dev->bars[5];
  printk(PRINTK_INFO "AHCI: AHCI HBA found on PCI bus %d, slot %d\n", 
         dev->bus, dev->slot);
  

  /*
   *  The HBA needs to perform DMA
   *  so we must enable PCI bus
   *  mastering to do so.
   *
   */

  pci_enable_bus_mastering(dev);


  /*
   *  BIOS has control over the HBA.
   *  That will change once we do
   *  an HBA BIOS handoff.
   */

  printk(PRINTK_INFO "AHCI: Performing HBA BIOS handoff..\n");
  if (take_ownership() != 0)
  {
    return;
  }

  abar->ghc |=  (1 << 31);        /* Enable AHCI */
  abar->ghc &= ~(1 << 1);         /* Clear interrupts enabled bit */


  printk(PRINTK_INFO "AHCI: Verifying HBA capabilities..\n");
  if (read_hba_cap() != 0)
  {
    return;
  }

  init_ports();
}
