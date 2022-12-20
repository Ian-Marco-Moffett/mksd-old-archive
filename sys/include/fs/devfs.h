#ifndef FS_DEVFS_H_
#define FS_DEVFS_H_

#include <fs/vfs.h>


void devfs_register_device(const char* id, fops_t* fops);
void devfs_init(void);


#endif
