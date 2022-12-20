/*
 *  Description: Block device interface.
 *  Author: Ian Marco Moffett.
 *
 */


#include <block/dev.h>
#include <drivers/hdd/ahci.h>
#include <fs/vfs.h>


static const char* devstr[] = {
  [BDEV_TYPE_HDD] = "/dev/sda",
  [BDEV_TYPE_SSD] = "/dev/nvme"
};


int
block_read_drive(bdevtype_t requested_blockdev, uint16_t* buf_phys, uint8_t sector_count, uint64_t lba)
{
  if (requested_blockdev > BDEV_TYPE_TOP || requested_blockdev < 0)
  {
    return 1;
  }

  FILE* fp = fopen(devstr[requested_blockdev], "r");
  if (fp == NULL)
  {
    fclose(fp);
    return 1;
  }
  fclose(fp);

  /* TODO: Implement NVMe */
  switch (requested_blockdev)
  {
    case BDEV_TYPE_HDD:
      ahci_read_drive(lba, buf_phys, sector_count);
      break;
  }

  return 0;
}

int 
block_write_drive(bdevtype_t requested_blockdev, uint16_t* buf_phys, uint8_t sector_count, uint64_t lba)
{
  if (requested_blockdev > BDEV_TYPE_TOP || requested_blockdev < 0)
  {
    return 1;
  }

  FILE* fp = fopen(devstr[requested_blockdev], "r");
  if (fp == NULL)
  {
    fclose(fp);
    return 1;
  }
  fclose(fp);

  /* TODO: Implement NVMe */
  switch (requested_blockdev)
  {
    case BDEV_TYPE_HDD:
      ahci_write_drive(lba, buf_phys, sector_count);
      break;
  }

  return 0;
}
