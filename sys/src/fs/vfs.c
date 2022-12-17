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

static vfs_node_t* root_node = NULL;


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
  vfs_node_t* last_node = root_node;

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

  return last_node == root_node ? NULL : last_node;
}


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
  new_node->is_dir = is_dir;
  memcpy(new_node->name, name, strlen(name));

  if (parent != NULL)
  {
    hashmap_insert(&parent->children, new_node, name);
  }

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
}
