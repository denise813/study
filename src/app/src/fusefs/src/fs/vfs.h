#ifndef VFS_H
#define VFS_H

#include "vfs_op.h"
#include "src/block/vblock.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct vfs
{
    int vfs_fs_type;
    void * vfs_private;
    vfs_syscall_t * vfs_op;
    vblock_t * vfs_block;
}vfs_t;


int vfs_malloc_fs(char * fs_name, vfs_t **fs);
void vfs_free_fs(vfs_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
