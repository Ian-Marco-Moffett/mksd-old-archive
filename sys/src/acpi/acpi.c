/*
 *  Description: ACPI module.
 *  Author(s): Ian Marco Moffett
 *
 */

#include <acpi/acpi.h>
#include <acpi/def.h>
#include <lib/log.h>
#include <lib/assert.h>
#include <lib/limine.h>
#include <lib/string.h>
#include <lib/asm.h>
#include <arch/x64/io.h>

static acpi_rsdt_t* rsdt = NULL;
static acpi_fadt_t* fadt = NULL;
static uint16_t SLP_EN;
static uint16_t SCI_EN;
static uint16_t SLP_TYPa;
static uint16_t SLP_TYPb;
static size_t rsdt_entry_count = 0;

static volatile struct limine_rsdp_request rsdp_req = {
  .id = LIMINE_RSDP_REQUEST,
  .revision = 0
};


static void locate_fadt(void) {
  for (size_t i = 0; i < rsdt_entry_count; ++i) {
    acpi_header_t* current = (acpi_header_t*)(uint64_t)rsdt->tables[i];
    if (memcmp(current->signature, "FACP", 4) == 0) {
      fadt = (acpi_fadt_t*)current;
      return;
    }
  }
}


static uint8_t do_checksum(const acpi_header_t* header) {
  uint8_t sum = 0;

  for (uint32_t i = 0; i < header->length; ++i) {
    sum += ((char*)header)[i];
  }

  return sum % 0x100 == 0;
}

static void parse_dsdt_s5(uintptr_t dsdt) {
  char* s5 = (char*)dsdt + 36;      // Skip the header.
  size_t dsdt_length = *((uint32_t*)(uint64_t)fadt->dsdt+1)-36;

  while (dsdt_length > 0) {
    if (memcmp(s5, "_S5_", 4) == 0) {
      break;
    }

    ++s5;
  }

  if (dsdt_length > 0) {
    if ((*(s5-1) == 0x08 || 
        (*(s5-2) == 0x08 && 
         *(s5-1) == '\\')) && *(s5+4) == 0x12) {
      printk(PRINTK_INFO "ACPI: Found \\_S5 at %x\n", s5);
      s5 += 5;
      s5 += ((*s5 & 0xC0) >> 16) + 2;     /* Calculate package length */

      if (*(s5) == 0xA) {
        // Skip the byte prefix.
        ++s5;
      }

      SLP_TYPa = *(s5) << 10;
      ++s5;

      if (*(s5) == 0xA) {
        // Skip the byte prefix again.
        ++s5;
      }

      SLP_TYPb = *(s5) << 10;
      SLP_EN = 1 << 13;
      SCI_EN = 1;

    } else {
      printk(PRINTK_WARN "ACPI: Failed to parse \\_S5.\n");
    }
  } else {
    printk(PRINTK_WARN "ACPI: \\_S5 not present!!\n");
  }
}

void acpi_power_off(void) {
  outw((uint32_t)fadt->pm1a_control_block, SLP_TYPa | SLP_EN);

  if (fadt->pm1b_control_block != 0) {
    outw(fadt->pm1a_control_block, SLP_TYPb | SLP_EN);
  }

  printk(PRINTK_WARN 
         "ACPI: Automatic shutdown failed!\n"
         "It is safe to manually turn off "
         "your computer now.");

  asmv("cli; hlt");
}

void acpi_init(void) {
  printk(PRINTK_INFO "ACPI: Setting up..\n");
  
  /* Fetch the RSDP */
  acpi_rsdp_t* rsdp = rsdp_req.response->address;
  
  /* Fetch the RSDT */
  rsdt = (acpi_rsdt_t*)(uint64_t)rsdp->rsdtaddr;
  ASSERT(do_checksum(&rsdt->header), "ACPI RSDT checksum invalid!\n");
  
  /* Get the entry count */
  rsdt_entry_count = (rsdt->header.length - sizeof(rsdt->header)) / 4;
  printk(PRINTK_INFO "ACPI: RSDT contains %d entries.\n", 
         rsdt_entry_count);

  /* Locate FADT */
  printk(PRINTK_INFO "ACPI: Locating ACPI FADT..\n");
  locate_fadt();

  ASSERT(fadt != NULL, "Could not locate ACPI FADT!\n");

  ASSERT(do_checksum(&fadt->header), "ACPI FADT checksum invalid!\n");

  printk(PRINTK_INFO "ACPI: FADT located at %x\n", fadt);
  printk(PRINTK_INFO "ACPI: ACPI DSDT location %x\n", fadt->dsdt);

  parse_dsdt_s5(fadt->dsdt);
}

