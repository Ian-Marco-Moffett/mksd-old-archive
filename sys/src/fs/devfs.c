#include <fs/devfs.h>
#include <lib/log.h>
#include <lib/asm.h>


static vfs_node_t* devfs;

void 
devfs_register_device(const char* id, fops_t* fops)
{
  _unused vfs_node_t* unused;
  vfs_make_node(id, devfs, &unused, 0, fops);
}

void 
devfs_init(void)
{
  vfs_make_node("dev", g_rootfs, &devfs, 0, NULL);
  printk(PRINTK_INFO "Mounted devfs on '/dev/'.\n");
}
