/*
 *  Description: MKSD Virtual Filesystem
 *  Author(s): Ian Marco Moffett
 *
 */

#include <fs/vfs.h>
#include <mm/heap.h>
#include <lib/string.h>
#include <lib/log.h>
#include <sys/errno.h>

static vfs_node_t* root_node = NULL;



int
vfs_make_node(const char* name, vfs_node_t* parent, 
              uint8_t is_dir, vfs_node_t** node_out)
{
  /* Check if the name is good enough */
  if (strlen(name) > 255)
  {
    return -ENAMETOOLONG;
  }
  
  *node_out = kmalloc(sizeof(vfs_node_t));
  vfs_node_t* new_node = *node_out;

  new_node->n_children = 0;
  new_node->parent = parent;
  memcpy(new_node->name, name, strlen(name));
  return 0;
}


void 
vfs_init(void)
{
  if (root_node != NULL)
  {
    return;
  }

  vfs_make_node("", NULL, 1, &root_node);
  printk(PRINTK_INFO "Mounted '/'\n");
}
