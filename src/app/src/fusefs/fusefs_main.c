#include <stdio.h>
#include "src/ysfs_log.h"
#include "src/ysfs_config.h"
#include "src/fuse/fusefs_fuse.h"


int trans_fuse_config(ysfs_config_t * fs_config, fusefs_fuse_fs_config_t * fuse_config)
{
    fuse_config->ffs_bdev = fs_config->ysc_dev;
    fuse_config->ffs_fsname = fs_config->ysc_fs_name;
    fuse_config->ffs_mountpoint = fs_config->ysc_mount_point;

    return 0;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    ysfs_config_t * fs_config = NULL;
    fusefs_fuse_fs_config_t fuse_config;
    fusefs_fuse_fs_t * fs = NULL;

    rc = malloc_ys_config(&fs_config);
    if (rc < 0) {
        goto l_out;
    }

    rc = cmd_parse_configure(argc, argv, fs_config);
    if (rc < 0) {
        goto l_free_config;
    }

    if (fs_config->ysc_has_help) {
        rc = 0;
        cmd_help_configure();
        goto l_free_config;
    }

    trans_fuse_config(fs_config, &fuse_config);
    fusefs_malloc_fs(&fuse_config, &fs);
    fs->init(fs);
    fs->mount(fs);
    fs->run(argc, argv, fs);
    fs->umount(fs);
    fs->exit(fs);
    fusefs_free_fs(fs);
    free_ys_config(fs_config);
    rc = 0;

l_out:
    return rc;
l_free_config:
    free_ys_config(fs_config);
    fs_config = NULL;
    goto l_out;
}
