/*
 *  Description: Functions for PCI access.
 *  Author(s): Ian Marco Moffett
 *
 */

#include <arch/bus/pci.h>
#include <arch/x64/io.h>
#include <mm/heap.h>

/* For configuration space access */
#define CONFIG_ADDR 0xCF8
#define CONFIG_DATA 0xCFC


static inline uint32_t
pci_get_cfg_addr(uint32_t bus, uint32_t slot, uint32_t func, 
                 uint8_t offset)
{
  return (uint32_t)((1 << 31)           /* Enable bit */
                    | (bus << 16)
                    | (slot << 11)
                    | (func << 8)
                    | (offset & 0xFC));
}

static inline uint8_t 
pci_read_irq_line(uint32_t bus, uint32_t slot, uint32_t func)
{
  return pci_config_readw(bus, slot, func, 0x3C) & 0xFF;
}


static inline uint16_t 
pci_read_vendor(uint8_t bus, uint8_t slot, uint8_t func)
{
  return pci_config_readw(bus, slot, func, 0x0);
}


static inline uint16_t
pci_read_device_id(uint8_t bus, uint8_t slot, uint8_t func)
{
  return pci_config_readw(bus, slot, func, 0x2);
}


static inline uint8_t 
pci_read_device_class(uint8_t bus, uint8_t slot, uint8_t func)
{
  return pci_config_readw(bus, slot, func, 0xA) >> 8;
}

static inline uint8_t 
pci_read_device_subclass(uint8_t bus, uint8_t slot, uint8_t func)
{
  return pci_config_readw(bus, slot, func, 0xA) & 0xFF;
}

static inline uint8_t 
pci_read_progif(uint8_t bus, uint8_t slot, uint8_t func)
{
  return pci_config_readw(bus, slot, func, 0x8) >> 8;
}

static inline uint32_t 
get_bar0(uint8_t bus, uint8_t slot, uint8_t func) 
{
  uint16_t lo = pci_config_readw(bus, slot, func, 0x10);
  uint16_t hi = pci_config_readw(bus, slot, func, 0x12);
  return ((uint32_t)hi << 16 | lo);
}

static inline uint32_t 
get_bar1(uint8_t bus, uint8_t slot, uint8_t func) 
{
  uint16_t lo = pci_config_readw(bus, slot, func, 0x14);
  uint16_t hi = pci_config_readw(bus, slot, func, 0x16);
  return ((uint32_t)hi << 16 | lo);
}


static inline uint32_t 
get_bar2(uint8_t bus, uint8_t slot, uint8_t func) 
{
  uint16_t lo = pci_config_readw(bus, slot, func, 0x18);
  uint16_t hi = pci_config_readw(bus, slot, func, 0x1A);
  return ((uint32_t)hi << 16 | lo);
}


static inline uint32_t 
get_bar3(uint8_t bus, uint8_t slot, uint8_t func) 
{
  uint16_t lo = pci_config_readw(bus, slot, func, 0x1C);
  uint16_t hi = pci_config_readw(bus, slot, func, 0xE);
  return ((uint32_t)hi << 16 | lo);
}

static inline uint32_t 
get_bar4(uint8_t bus, uint8_t slot, uint8_t func) 
{
  uint16_t lo = pci_config_readw(bus, slot, func, 0x20);
  uint16_t hi = pci_config_readw(bus, slot, func, 0x22);
  return ((uint32_t)hi << 16 | lo);
}

static inline uint32_t 
get_bar5(uint8_t bus, uint8_t slot, uint8_t func) 
{
  uint16_t lo = pci_config_readw(bus, slot, func, 0x24);
  uint16_t hi = pci_config_readw(bus, slot, func, 0x26);
  return ((uint32_t)hi << 16 | lo);
}

static inline void 
pci_init_dev(pci_device_t* dev, uint8_t bus, uint8_t slot, uint8_t func)
{
  dev->bars[0] = get_bar0(bus, slot, func);
  dev->bars[1] = get_bar1(bus, slot, func);
  dev->bars[2] = get_bar2(bus, slot, func);
  dev->bars[3] = get_bar3(bus, slot, func);
  dev->bars[4] = get_bar4(bus, slot, func);
  dev->bars[5] = get_bar5(bus, slot, func);
  dev->bus = bus;
  dev->slot = slot;
  dev->func = func;
}

uint16_t 
pci_config_readw(uint32_t bus, uint32_t slot, uint32_t func, 
                 uint8_t offset)
{
  outl(CONFIG_ADDR, pci_get_cfg_addr(bus, slot, func, offset));
  return (uint16_t)((inl(CONFIG_DATA) >> ((offset & 2) * 8)) & 0xFFFF);
}

void
pci_config_writew(uint32_t bus, uint32_t slot, uint32_t func, 
                  uint8_t offset, uint16_t data)
{
  outl(CONFIG_ADDR, pci_get_cfg_addr(bus, slot, func, offset));
  outl(CONFIG_DATA, data);
}


void 
pci_enable_bus_mastering(pci_device_t* dev)
{
  uint16_t val = pci_config_readw(dev->bus, dev->slot, dev->func, 0x4);
  pci_config_writew(dev->bus, dev->slot, dev->func, 0x4, (val 
                                                          | (1 << 2) 
                                                          | (1 << 0)));
}


pci_device_t*
pci_find(uint16_t vendor_id, uint16_t device_id)
{
  pci_device_t* dev = NULL;

  for (uint8_t bus = 0; bus < 5; ++bus)
  {
    for (uint8_t slot = 0; slot < 32; ++slot)
    {
      for (uint8_t func = 0; func < 8; ++func)
      {
        if (pci_read_vendor(bus, slot, func) == vendor_id 
            && pci_read_device_id(bus, slot, func) == device_id)
        {
          dev = kmalloc(sizeof(pci_device_t));
          pci_init_dev(dev, bus, slot, func);
          return dev;
        }
      }
    }
  }

  return NULL;
}


pci_device_t* 
pci_find_any(uint8_t class, uint8_t subclass,
                           int8_t interface) 
{
  pci_device_t* dev = NULL;

  for (uint8_t bus = 0; bus < 5; ++bus)
  {
    for (uint8_t slot = 0; slot < 32; ++slot)
    {
      for (uint8_t func = 0; func < 8; ++func)
      {
        if (pci_read_device_class(bus, slot, func) == class
            && pci_read_device_subclass(bus, slot, func) == subclass)
        {
          if (pci_read_progif(bus, slot, func) != interface 
              && interface != -1)
          {
            continue;
          }

          dev = kmalloc(sizeof(pci_device_t));
          pci_init_dev(dev, bus, slot, func);
          return dev;
        }
      }
    }
  }

  return NULL;
}
