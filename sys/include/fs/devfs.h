#ifndef DEVFS_H_
#define DEVFS_H_

#include <fs/vfs.h>


void devfs_register_device(const char* dev_name, fops_t* ops,
                           vfs_flags_t flags);

void devfs_init(void);


#endif
