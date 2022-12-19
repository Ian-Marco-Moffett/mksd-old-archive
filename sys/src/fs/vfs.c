#include <fs/vfs.h>
#include <lib/string.h>
#include <lib/log.h>

static vfs_node_t rootfs;


static vfs_node_t*
vfs_path_to_node(const char* path)
{
  if (*path != '/')
  {
    return NULL;
  }
  ++path;

  vfs_node_t* last_node = &rootfs; 
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

  if (last_node != &rootfs)
  {
    if (should_be_dir && !(last_node->flags & FILE_FLAG_DIR))
    {
      return NULL;
    }
  }

  return last_node == &rootfs ? NULL : last_node;
}


void
vfs_make_node(const char* name, vfs_node_t* parent,
              vfs_node_t* node_out, file_flag_t flags)
{
  memcpy(node_out->name, name, strlen(name));
  node_out->n_children = 0;
  node_out->flags = flags;
  node_out->children = kmalloc(sizeof(hashmap_t));

  if (parent != NULL)
  {
    hashmap_insert(parent->children, node_out, name);
    ++parent->n_children;
  }
}

void 
vfs_init(void)
{  
  vfs_make_node("", NULL, &rootfs, FILE_FLAG_DIR); 
}
