#ifndef HDD_AHCI_H_
#define HDD_AHCI_H_

#include <lib/types.h>

typedef enum
{
  FIS_TYPE_REG_H2D = 0x27,      /* Host to device */
  FIS_TYPE_REG_D2H = 0x34,      /* Device to host */
  FIS_TYPE_DATA    = 0x46,      /* Data */
} FIS_TYPE;

typedef struct
{
  uint8_t fis_type;             /* FIS_TYPE_REG_H2D */
  uint8_t pmport : 4;           /* Port multiplier */
  uint8_t r0     : 3;           /* Reserved */
  uint8_t c      : 1;           /* 0: Control, 1: Command */
  uint8_t command;              /* Command register */
  uint8_t featurel;             /* Feature register (bits 7:0) */
  
  uint8_t lba0;                 /* LBA bits 7:0 */
  uint8_t lba1;                 /* LBA bits 15:8 */
  uint8_t lba2;                 /* LBA bits 23:16 */
  uint8_t device;               /* Device register */

  uint8_t lba3;                 /* LBA bits 31:24 */
  uint8_t lba4;                 /* LBA bits 39:32 */
  uint8_t lba5;                 /* LBA bits 47:40 */
  uint8_t featureh;             /* Feature register (bits 15:8) */

  uint8_t countl;               /* Count bits 7:0 */
  uint8_t counth;               /* Count bits 15:8 */
  uint8_t icc;                  /* Isochronous command completion */
  uint8_t control;              /* Control register */
  uint8_t r1[4];                /* Reserved */
} FIS_REG_H2D;


typedef struct
{
  uint8_t fis_type;             /* FIS_TYPE_DATA */
  uint8_t pmport : 4;           /* Port multiplier */
  uint8_t r0     : 4;           /* Reserved */
  uint8_t r1[2];                /* Reserved */
  uint32_t data[1];             /* Payload */
} FIS_DATA;


typedef volatile struct
{
  uint32_t clb;                 /* Command list base (must be 1K aligned) */
  uint32_t clbu;                /* Command list base upper 32 bits */
  uint32_t fb;                  /* FIS base (256 byte aligned) */
  uint32_t fbu;                 /* FIS base upper */
  uint32_t is;                  /* Interrupt status */
  uint32_t ie;                  /* Interrupt enable */
  uint32_t cmd;                 /* Command and status */
  uint32_t r0;                  /* Reserved */
  uint32_t tfd;                 /* Task file data */
  uint32_t sig;                 /* Signature */
  uint32_t ssts;                /* SATA status */
  uint32_t sctl;                /* SATA control */
  uint32_t serr;                /* SATA error */
  uint32_t sact;                /* SATA active */
  uint32_t ci;                  /* Command issue */
  uint32_t sntf;                /* SATA notification */
  uint32_t fbs;                 /* FIS-based switch control */
  uint32_t r1[11];              /* Reserved */
  uint32_t vendor[4];           /* Vendor specific */
} HBA_PORT;

typedef volatile struct
{
  uint32_t cap;                 /* Host capability */
  uint32_t ghc;                 /* Global host control */
  uint32_t is;                  /* Interrupt status */ 
  uint32_t pi;                  /* Port implemented */
  uint32_t vs;                  /* Version */
  uint32_t ccc_ctl;             /* Command completion coalescing control */
  uint32_t ccc_pts;             /* Command completion coalescing ports */
  uint32_t em_loc;              /* Enclosure management location */
  uint32_t em_ctl;              /* Enclosure management control */
  uint32_t cap2;                /* Host capabilities extended */
  uint32_t bohc;                /* BIOS/OS handoff control and status */
  uint8_t r0[0xA0-0x2C];        /* Reserved */
  uint8_t vendor[0x100-0xA0];   /* Vendor specific */
  HBA_PORT ports[1];            /* 1 ~ 32 */
} HBA_MEM;


typedef struct
{
  uint8_t cfl  : 5;              /* Command FIS length */
  uint8_t a    : 1;              /* ATAPI */
  uint8_t w    : 1;              /* Write */
  uint8_t p    : 1;              /* Prefetchable */
  uint8_t r    : 1;              /* Reset */
  uint8_t b    : 1;              /* BIST */
  uint8_t c    : 1;              /* Clear busy upon R_OK */
  uint8_t resv : 1;              /* Reserved */
  uint8_t pmp  : 4;              /* Port multiplier port */
  uint16_t prdtl;                /* Phys region descriptor tbl length */
  volatile uint32_t prdbc;       /* PRDT byte count transferred */
  uint32_t ctba;                 /* Command table desc base */
  uint32_t ctbau;                /* Command table desc base (upper 32 bits) */
  uint32_t r1[4];                /* Reserved */
} HBA_CMD_HEADER;

typedef struct
{
  uint32_t dba;         /* Data base addr */
  uint32_t dbau;        /* Data base addr upper */
  uint32_t r0;          /* Reserved */
  uint32_t dbc;         /* Byte count */
  uint32_t r1;          /* Reserved */
  uint32_t i : 1;       /* Interrupt on completion */
} HBA_PRDT_ENTRY;

typedef struct
{
  uint8_t cfis[64];      /* Command fis */
  uint8_t acmd[16];      /* ATAPI command */
  uint8_t r[48];         /* Reserved */
  HBA_PRDT_ENTRY prdt[1];
} HBA_CMD_TBL;


void ahci_init(void);

#endif
