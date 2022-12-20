/*
 *  Description: Ext2 filesystem implementation.
 *  Author(s): Ian Marco Moffett.
 *
 */


#include <fs/ext2.h>
#include <block/dev.h>
#include <mm/pmm.h>
#include <mm/vmm.h>
#include <lib/log.h>

static superblock_t* superblock = NULL;

typedef enum
{
  CREATOR_LINUX,
  CREATOR_GNU_HURD,
  CREATOR_MASIX,
  CREATOR_FREEBSD,
  CREATOR_OTHER
} fscreator_id_t;

const char* fscreators[] = {
  [CREATOR_LINUX] = "Linux",
  [CREATOR_GNU_HURD] = "GNU HURD",
  [CREATOR_MASIX] = "MASIX",
  [CREATOR_FREEBSD] = "FreeBSD",
  [CREATOR_OTHER] = "Unknown"
};

static void
dump_fsinfo(void)
{
  printk(PRINTK_INFO "EXT2: Total inodes: %d\n", superblock->total_inodes);
  printk(PRINTK_INFO "EXT2: Total blocks: %d\n", superblock->total_blocks);
  printk(PRINTK_INFO "EXT2: Filesystem created by a %s system.",
         fscreators[superblock->os_id]);
}

void 
ext2_init(void)
{
  uintptr_t superblock_alloc = pmm_alloc(1);
  if (block_read_drive(BDEV_TYPE_HDD, (void*)superblock_alloc, 2, 2) != 0)
  {
    printk(PRINTK_WARN "EXT2: Could not read from HDD (aborting).\n");
    printk(PRINTK_WARN "EXT2: /dev/sda (No such file or directory)\n");
    return;
  }

  superblock = (superblock_t*)(superblock_alloc + VMM_HIGHER_HALF);
  if (superblock->ext2_magic != 0xEF53)
  {
    printk(PRINTK_WARN "EXT2: This is not an ext2 filesystem!\n");
    return;
  }

  printk(PRINTK_INFO "EXT2: Found ext2 filesystem on /dev/sda\n");
  dump_fsinfo();
}
