#ifndef FS_VFS_H_
#define FS_VFS_H_


#include <lib/hashmap.h>
#include <lib/types.h>

#define MAX_FILENAME_LENGTH 255

#define VFS_FLAG_READ       (1 << 0)  /* Readable */
#define VFS_FLAG_WRITE      (1 << 1)  /* Writable */
#define VFS_FLAG_DIRECTORY  (1 << 2)  /* Directory */

typedef uint8_t vfs_flags_t;
struct VFSNode;

typedef struct 
{
  void(*open)(struct VFSNode* vfsnode);
  void(*close)(struct VFSNode* vfsnode);
} fops_t;


typedef struct VFSNode
{
  char name[256];
  struct VFSNode* parent;
  size_t n_children;
  hashmap_t children;
  fops_t* fs_ops;
  vfs_flags_t flags;
} vfs_node_t;


void vfs_init(void);

/*
 *  @param name: Name of the file.
 *  @param parent: File's parent.
 *  @param is_dir: Must be 1 if is directory.
 *  @param node_out: The newly created node is outputted here.
 *  @returns: An errno value.
 *
 */

int vfs_make_node(const char* name, vfs_node_t* parent, 
                  vfs_flags_t flags, vfs_node_t** node_out,
                  fops_t* fs_ops);

void vfs_print_perms(const char* fsname, const vfs_node_t* node);

extern vfs_node_t* g_root_fs;
#endif
