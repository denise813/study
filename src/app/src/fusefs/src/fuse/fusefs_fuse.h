#ifndef FUSEFS_FUSE_H
#define FUSEFS_FUSE_H

#include "fusefs_fuse_def.h"
#include "src/fs/vfs.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct fusefs_fuse_blockdev_config
{
    char * fb_bdev;
}fusefs_fuse_blockdev_config_t;


typedef struct fusefs_fuse_blockdev
{
    fusefs_fuse_blockdev_config_t * fbdev_config;
    int (*open)(struct fusefs_fuse_blockdev* bdev);
    int (*close)(struct fusefs_fuse_blockdev* bdev);
    int (*init)(struct fusefs_fuse_blockdev* bdev);
    int (*exit)(struct fusefs_fuse_blockdev* bdev);
    int (*read)(struct fusefs_fuse_blockdev* bdev, off_t off, size_t read_len, char * buff, size_t buf_len);
    int (*write)(struct fusefs_fuse_blockdev* bdev, off_t off, size_t write_len, char * buff, size_t buf_len);
}fusefs_fuse_blockdev_t;


typedef struct fusefs_fuse_fs_config
{
    char * ffs_bdev;
    char * ffs_mountpoint;
    char * ffs_fsname;
}fusefs_fuse_fs_config_t;


typedef struct fusefs_fuse_fs
{
    fusefs_fuse_fs_config_t * ffs_config;
    fusefs_fuse_blockdev_t *  ffs_bdev;
    struct fuse_operations * ffs_op;
    vfs_t * ffs_vfs;
    int (*init)(struct fusefs_fuse_fs*);
    int (*run)(int, char *[], struct fusefs_fuse_fs*);
    int (*exit)(struct fusefs_fuse_fs*);
}fusefs_fuse_fs_t;


int fusefs_malloc_fs(fusefs_fuse_fs_config_t *, fusefs_fuse_fs_t**);
void fusefs_free_fs(fusefs_fuse_fs_t *);
fusefs_fuse_fs_t* fusefs_get_fs();
struct fuse_operations * fusefs_get_fs_op();

int fusedev_malloc_blockdev(fusefs_fuse_blockdev_config_t *, fusefs_fuse_blockdev_t**);
void fusedev_free_blockdev(fusefs_fuse_blockdev_t *);



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
