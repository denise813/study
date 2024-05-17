#ifndef VFS_H
#define VFS_H

#include "vfs_op.h"
#include "src/block/vblock.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct vfs_config
{
    int vfs_fs_type;
    char * vfs_dev;
}vfs_config_t;

typedef struct vfs
{
    vfs_config_t vfs_config;
    void * vfs_private;
    vfs_syscall_t * vfs_op;
    vblock_t * vfs_block;
}vfs_t;


int vfs_malloc_fs(vfs_config_t * config, vfs_t **fs);
void vfs_free_fs(vfs_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
