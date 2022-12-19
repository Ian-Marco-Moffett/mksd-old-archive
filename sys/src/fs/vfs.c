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
#include <lib/asm.h>        // XXX: Remove this include after using
                            // vfs_path_to_node.

vfs_node_t* g_root_fs = NULL;


static inline vfs_node_t*
vfs_name_to_node(const char* name, vfs_node_t* in)
{
  return hashmap_read(&in->children, name);
}

// XXX: Remove the _unused later on.
_unused static vfs_node_t*
vfs_path_to_node(const char* path)
{
  if (*path != '/')
  {
    /* Path must must start with '/' */
    return NULL;
  }
  
  /* Skip '/' */
  ++path;
  
  /* Current filename */
  char current_fname[256];
  size_t cur_fname_idx = 0;

  /* Last node we found */
  vfs_node_t* last_node = g_root_fs;

  for (size_t i = 0; i < strlen(path); ++i)
  {
    if (path[i] == '/')
    {
      current_fname[cur_fname_idx] = '\0';
      last_node = vfs_name_to_node(current_fname, last_node);
    }

    if (path[i + 1] == '\0')
    {
      break;
    }

    current_fname[cur_fname_idx++] = path[i];
  }

  return last_node == g_root_fs ? NULL : last_node;
}


int 
vfs_make_node(const char* name, vfs_node_t* parent, 
                  vfs_flags_t flags, vfs_node_t** node_out,
                  fops_t* fs_ops)
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
  new_node->fs_ops = fs_ops;
  new_node->flags = flags;
  memcpy(new_node->name, name, strlen(name));

  if (parent != NULL)
  {
    hashmap_insert(&parent->children, new_node, name);
  }

  return 0;
}


void 
vfs_print_perms(const char* fsname, const vfs_node_t* node)
{
  printk(PRINTK_INFO "%s flags: -%c%c%c-\n", fsname,
         node->flags & (VFS_FLAG_READ)      ? 'r' : '-',
         node->flags & (VFS_FLAG_WRITE)     ? 'w' : '-',
         node->flags & (VFS_FLAG_DIRECTORY) ? 'd' : '-');
}


void 
vfs_init(void)
{
  if (g_root_fs != NULL)
  {
    return;
  }

  vfs_make_node("", NULL, 1, &g_root_fs, NULL);
}
