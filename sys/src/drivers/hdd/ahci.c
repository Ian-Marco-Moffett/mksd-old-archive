/*
 *  Description: AHCI SATA driver.
 *  Author(s): Ian Marco Moffett
 *
 */



#include <drivers/hdd/ahci.h>
#include <arch/bus/pci.h>
#include <lib/log.h>
#include <lib/asm.h>
#include <mm/pmm.h>
#include <mm/vmm.h>

#define CLASS_ID    0x1
#define SUBCLASS_ID 0x6

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

#define SATA_SIG_ATA 0x00000101     /* SATA drive */
#define SATA_SIG_ATAPI 0xEB140101   /* SATAPI drive */
#define SATA_SIG_SEMB 0xC33C0101    /* Enclosure management bridge */
#define SATA_SIG_PM 0x96690101      /* Port multiplier */

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM 3
#define AHCI_DEV_SATAPI 4



static pci_device_t* dev = NULL;
static size_t n_hba_ports = 0;
static volatile HBA_MEM* abar = NULL;


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

  /* Dump some information */
  get_port_info(port);
  
  /* Start up the command engine */
  printk(PRINTK_INFO "AHCI: Starting up HBA command engine..\n");
  start_cmd_engine(port);
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
          printk(PRINTK_INFO "AHCI: Port %d initialized successfully!\n", i);
          break;
      }
    }
  }
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
