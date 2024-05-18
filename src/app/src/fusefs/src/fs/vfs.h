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

typedef struct agent_fs
{
    void * afs_private;
    vfs_syscall_t * afs_op;
}agent_fs_t;

typedef struct vfs
{
    vfs_config_t vfs_config;
    agent_fs_t vfs_agentfs;
    vfs_syscall_t * vfs_op;
    vblock_t * vfs_block;
}vfs_t;

typedef struct vfs_inode
{
    void * f_private;
}vfs_info_t;


typedef struct vfs_file
{
     void * f_private;
}vfs_file_t;

typedef struct vfs_dir
{
     //struct dirent * d_itor;
     void * d_private;
}vfs_dir_t;



int vfs_malloc_fs(vfs_config_t * config, vfs_t **fs);
void vfs_free_fs(vfs_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
