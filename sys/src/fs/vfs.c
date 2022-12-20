/*
 *  Description: Virtual file system abstraction.
 *  Author(s): Ian Marco Moffett
 *
 */


#include <fs/vfs.h>
#include <lib/string.h>
#include <lib/log.h>

vfs_node_t* g_rootfs;


static vfs_node_t*
vfs_path_to_node(const char* path)
{
  if (*path != '/')
  {
    return NULL;
  }
  ++path;

  vfs_node_t* last_node = g_rootfs; 
  size_t dilm_count = strdilm_count(path, '/');
  uint8_t should_be_dir = 1;
  
  if (path[strlen(path) - 1] != '/')
  {
    ++dilm_count;
    should_be_dir = 0;
  }

  for (size_t i = 0; i < dilm_count; ++i)
  {
    char* split = strsplit(path, '/', i);
    last_node = hashmap_read(last_node->children, split);
    kfree(split);
  }

  if (last_node != g_rootfs)
  {
    if (should_be_dir && !(last_node->flags & FILE_FLAG_DIR))
    {
      return NULL;
    }
  }

  return last_node == g_rootfs ? NULL : last_node;
}


void vfs_make_node(const char* name, vfs_node_t* parent, 
                   vfs_node_t** node_out, file_flag_t flags,
                   fops_t* fops)
{
  *node_out = kmalloc(sizeof(vfs_node_t));
  vfs_node_t* new_node = *node_out;
  memcpy(new_node->name, name, strlen(name));
  new_node->n_children = 0;
  new_node->flags = flags;
  new_node->fops = fops;
  new_node->children = kmalloc(sizeof(hashmap_t));

  if (parent != NULL)
  {
    hashmap_insert(parent->children, new_node, name);
    ++parent->n_children;
  }
}

FILE* 
fopen(const char* path, const char* mode)
{
  uint8_t flags = 0;
  if (strcmp(mode, "r") == 0)
  {
    flags = FOPEN_FLAG_READ;
  }
  else
  {
    return NULL;
  }

  vfs_node_t* node = vfs_path_to_node(path);
  node->fops->open(node);
  
  FILE* fp = kmalloc(sizeof(FILE));
  fp->node = node;
  fp->flags = flags;
  return fp;
}

void 
vfs_init(void)
{
  vfs_make_node("", NULL, &g_rootfs, FILE_FLAG_DIR, NULL);
}
