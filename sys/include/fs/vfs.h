#ifndef FS_VFS_H_
#define FS_VFS_H_


#include <lib/hashmap.h>
#include <lib/types.h>


typedef struct VFSNode
{
  char name[256];
  struct VFSNode* parent;
  size_t n_children;
  hashmap_t* children;
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
                  uint8_t is_dir, vfs_node_t** node_out);


#endif
