#include <fs/devfs.h>
#include <fs/vfs.h>
#include <lib/log.h>
#include <lib/asm.h>

static vfs_node_t* devfs = NULL;

void devfs_register_device(const char* dev_name, fops_t* ops,
                           vfs_flags_t flags)
{
  _unused vfs_node_t* unused = NULL;
  vfs_make_node(dev_name, devfs, flags, &unused, ops);
}

void 
devfs_init(void)
{
  vfs_make_node("dev", g_root_fs, VFS_FLAG_DIRECTORY 
                                  | VFS_FLAG_READ, 
                                    &devfs, NULL);
  printk(PRINTK_INFO "Mounted devfs on '/dev'\n");
  vfs_print_perms("devfs", devfs);
}
