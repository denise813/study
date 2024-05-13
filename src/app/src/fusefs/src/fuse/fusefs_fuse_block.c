#include <stdlib.h>
#include "fusefs_fuse.h"


int fusedev_fuse_open(struct fusefs_fuse_blockdev* bdev)
{
    return 0;
}

int fusedev_fuse_close(struct fusefs_fuse_blockdev * bdev)
{
    return 0;
}

int fusedev_fuse_init(struct fusefs_fuse_blockdev * bdev)
{
    return 0;
}

int fusedev_fuse_exit(struct fusefs_fuse_blockdev * bdev)
{
    return 0;
}

int fusedev_fuse_read(
                struct fusefs_fuse_blockdev * bdev,
                off_t off,
                size_t read_len,
                char * buff,
                size_t buf_len)
{
    return 0;
}

int fusedev_fuse_write(
            struct fusefs_fuse_blockdev* bdev,
            off_t off,
            size_t write_len,
            char * buff,
            size_t buf_len)
{
    return 0;
}


int malloc_fusefs_fuse_blockdev(fusefs_fuse_blockdev_config_t * config, fusefs_fuse_blockdev_t** bdev)
{
    fusefs_fuse_blockdev_t * entry = NULL;

    entry = (fusefs_fuse_blockdev_t*)malloc(sizeof(fusefs_fuse_blockdev_t));
    entry->close = fusedev_fuse_close;
    entry->exit = fusedev_fuse_exit;
    entry->init = fusedev_fuse_init;
    entry->open = fusedev_fuse_open;
    entry->read = fusedev_fuse_read;
    entry->write = fusedev_fuse_write;
    entry->fbdev_config = config;

    *bdev = entry;
    return 0;
}

void free_fusefs_fuse_blockdev(fusefs_fuse_blockdev_t * bdev)
{
     bdev->fbdev_config = NULL;
     free(bdev);

    return;
}



