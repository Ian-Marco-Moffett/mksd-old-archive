#ifndef BUS_PCI_H_
#define BUS_PCI_H_

#include <lib/types.h>

typedef struct
{
  uintptr_t bars[5];
  uint8_t bus;
  uint8_t slot;
  uint8_t func;
  uint8_t irq_line;
} pci_device_t;

/*
 *  Reads a word from the PCI configuration space.
 *
 */
uint16_t pci_config_readw(uint32_t bus, uint32_t slot, uint32_t func, uint8_t offset);

/*
 * Writes a word to the PCI configuration space.
 *
 */

void pci_config_writew(uint32_t bus, uint32_t slot, uint32_t func, uint8_t offset,
                           uint16_t data);


/*
 *  Enables bus mastering for a device
 *  on the PCI bus.
 *
 *  This can be used so the device can
 *  perform DMA.
 *
 */

void pci_enable_bus_mastering(pci_device_t* dev);

#endif
