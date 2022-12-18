/*
 *  Description: AHCI SATA driver.
 *  Author(s): Ian Marco Moffett
 *
 */



#include <drivers/hdd/ahci.h>
#include <arch/bus/pci.h>
#include <lib/log.h>
#include <lib/asm.h>

#define CLASS_ID    0x1
#define SUBCLASS_ID 0x6



static pci_device_t* dev = NULL;
static size_t n_hba_ports = 0;
static volatile HBA_MEM* abar = NULL;

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

  printk(PRINTK_INFO "AHCI: performing HBA BIOS handoff..\n");
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
}
