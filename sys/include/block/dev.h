#ifndef BLOCK_DEV_H_
#define BLOCK_DEV_H_

#include <lib/types.h>



typedef enum
{
  BDEV_TYPE_SSD,
  BDEV_TYPE_HDD,
  BDEV_TYPE_TOP,        /* Unused */
} bdevtype_t;

/*
 *  @param: requested_blockdev: The type of block device that should be read
 *                              from. If the function cannot find it then it
 *                              shall return a nonzero value.
 *
 *  @param: buf_phys: A buffer to read contents in 
 *                    (MUST BE A PHYSICAL ADDRESS).
 *
 *  @param sector_count: The amount of sectors to for this operation.
 */

int block_read_drive(bdevtype_t requested_blockdev, uint16_t* buf_phys, uint8_t sector_count, uint64_t lba);

/*
 *  Same as above but writes
 *  to the drive instead.
 */

int block_write_drive(bdevtype_t requested_blockdev, uint16_t* buf_phys, uint8_t sector_count, uint64_t lba);

#endif
