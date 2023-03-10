#ifndef BUS_PCI_H_
#define BUS_PCI_H_

#include <lib/types.h>

typedef struct
{
  uintptr_t bars[6];
  uint8_t bus;
  uint8_t slot;
  uint8_t func;
  uint8_t irq_line;
} pci_device_t;

/*
 *  Reads a word from the PCI configuration space.
 *
 */
uint16_t pci_config_readw(uint32_t bus, uint32_t slot, uint32_t func,
                          uint8_t offset);

/*
 * Writes a word to the PCI configuration space.
 *
 */

void pci_config_writew(uint32_t bus, uint32_t slot, uint32_t func,
                       uint8_t offset, uint16_t data);


/*
 *  Enables bus mastering for a device
 *  on the PCI bus.
 *
 *  This can be used so the device can
 *  perform DMA.
 *
 */

void pci_enable_bus_mastering(pci_device_t* dev);


pci_device_t* pci_find(uint16_t vendor_id, uint16_t device_id);

/*
 *  Set interface -1 for it to be ignored.
 *
 */

pci_device_t* pci_find_any(uint8_t class, uint8_t subclass,
                           int8_t interface);

#endif
