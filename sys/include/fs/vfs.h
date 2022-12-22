#ifndef FS_VFS_H_
#define FS_VFS_H_

#include <lib/types.h>
#include <lib/hashmap.h>


#define VFS_FILENAME_MAX_LEN 255
#define FILE_FLAG_DIR (1 << 0)

#define FOPEN_FLAG_READ  (1 << 0)
#define FOPEN_FLAG_WRITE (1 << 1)

typedef uint8_t file_flag_t;

struct VFSNode;

typedef struct
{
  void(*open)(struct VFSNode* node);
  void(*close)(struct VFSNode* node);
} fops_t;

typedef struct VFSNode
{
  char name[256];
  struct hashmap_s children;
  size_t n_children;
  file_flag_t flags;
  fops_t* fops;
} vfs_node_t;

typedef struct
{
  vfs_node_t* node;
  uint8_t flags;
} FILE;

void vfs_init(void);

void vfs_make_node(const char* name, vfs_node_t* parent, 
                   vfs_node_t** node_out, file_flag_t flags,
                   fops_t* fops);

FILE* fopen(const char* path, const char* mode);
void fclose(FILE* fp);


extern vfs_node_t* g_rootfs;

#endif
