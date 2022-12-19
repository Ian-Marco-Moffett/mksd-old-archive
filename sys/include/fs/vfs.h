#ifndef FS_VFS_H_
#define FS_VFS_H_

#include <lib/types.h>
#include <lib/hashmap.h>


#define VFS_FILENAME_MAX_LEN 255

#define FILE_FLAG_DIR (1 << 0)

typedef uint8_t file_flag_t;

typedef struct VFSNode
{
  char name[256];
  hashmap_t* children;
  size_t n_children;
  file_flag_t flags;
} vfs_node_t;

void vfs_init(void);
void vfs_make_node(const char* name, vfs_node_t* parent, 
                   vfs_node_t* node_out, file_flag_t flags);

#endif
