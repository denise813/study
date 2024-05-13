#include <stdlib.h>
#include <string.h>
#include "fusefs_fuse_def.h"
#include "fusefs_fuse.h"


static fusefs_fuse_fs_t * g_fusefs = NULL;


static int fusefs_fuse_register_bdev(struct fusefs_fuse_fs * fs)
{
    int rc = 0;
    fusefs_fuse_blockdev_t * bdev = NULL;
    fusefs_fuse_blockdev_config_t * cbdev = NULL;

    cbdev = (fusefs_fuse_blockdev_config_t*)malloc(sizeof(fusefs_fuse_blockdev_config_t));
    cbdev->fb_bdev = strdup(fs->ffs_config->ffs_bdev);
    
    malloc_fusefs_fuse_blockdev(cbdev, &bdev);

    fs->ffs_bdev = bdev;
    return 0;
}

static int fusefs_fuse_unregister_bdev(struct fusefs_fuse_fs * fs)
{
    if (fs->ffs_bdev->fbdev_config->fb_bdev) {
        free(fs->ffs_bdev->fbdev_config->fb_bdev);
    }
    if (fs->ffs_bdev->fbdev_config->fb_bdev) {
        free(fs->ffs_bdev->fbdev_config);
    }
    free_fusefs_fuse_blockdev(fs->ffs_bdev);
    return 0;
}

static int fusefs_fuse_mount(struct fusefs_fuse_fs * fs)
{
    fs->ffs_vfs->vfs_op->mount(fs->ffs_vfs);

    //return fusefs_mount(&fusefs, &config);
    return 0;
}

static int fusefs_fuse_unmount(struct fusefs_fuse_fs * fs)
{
    fs->ffs_vfs->vfs_op->unmount(fs->ffs_vfs);
    return 0;
}

static int fusefs_fuse_fs_init(struct fusefs_fuse_fs * fs)
{
    struct fuse_session *session = NULL;
    struct fuse_operations * fusefs_fuse_op = NULL;

    fusefs_fuse_op = get_fusefs_fuse_fs_op();
    session =  fuse_session_new(NULL, (const struct fuse_lowlevel_ops*)fusefs_fuse_op,
                    sizeof(struct fuse_operations), fs);
    fs->ffs_op = get_fusefs_fuse_fs_op();
    fs->ffs_session = session;

    fusefs_fuse_register_bdev(fs);
    malloc_vfs(fs->ffs_config->ffs_fsname, &fs->ffs_vfs);
    
   return 0;
}

static int fusefs_fuse_exit(struct fusefs_fuse_fs * fs)
{
    free_vfs(fs->ffs_vfs);
    fusefs_fuse_unregister_bdev(fs);
    fs->ffs_op = NULL;
    fuse_session_destroy(fs->ffs_session);
    fs->ffs_session = NULL;
   return 0;
}

static int fusefs_fuse_run(int argc, char *argv[], struct fusefs_fuse_fs * fs)
{
    int rc = 0;
    if (fuse_set_signal_handlers(fs->ffs_session) != 0) {
        goto l_out;
    }

    if (fuse_session_mount(fs->ffs_session, fs->ffs_config->ffs_mountpoint) != 0) {
        goto l_out;
    }

    rc= fuse_daemonize(0);
    if (fuse_set_signal_handlers(fs->ffs_session) != 0) {
        rc = 1;
        goto l_umount;
    }
    
    rc = fuse_session_loop(fs->ffs_session);
    //rc = fuse_session_loop_mt(fs->ffs_session, 0);
	if (rc) {
        rc = -1;
        goto l_umount;
       }

    fuse_remove_signal_handlers(fs->ffs_session);
	fuse_session_unmount(fs->ffs_session);
    rc = 0;

l_out:
   return rc;

l_remove_session:
    fuse_remove_signal_handlers(fs->ffs_session);
l_umount:
   fuse_session_unmount(fs->ffs_session);
    goto l_out;
}


int malloc_fusefs_fuse_fs(fusefs_fuse_fs_config_t * config, fusefs_fuse_fs_t** fs)
{
    fusefs_fuse_fs_t * entry = NULL;

    entry = (fusefs_fuse_fs_t*)malloc(sizeof(fusefs_fuse_fs_t));
    entry->ffs_config = config;
    //entry->ysffs_op = &g_fusefs_fuse_op;
    fusefs_fuse_register_bdev(entry);
    entry->mount = fusefs_fuse_mount;
    entry->umount = fusefs_fuse_unmount;
    entry->init = fusefs_fuse_fs_init;
    entry->exit = fusefs_fuse_exit;
    entry->run = fusefs_fuse_run;
    g_fusefs = entry;
    *fs = g_fusefs;
    return 0;
}

void free_fusefs_fuse_fs(fusefs_fuse_fs_t * fs)
{
    if (fs != NULL) {
        fusefs_fuse_unregister_bdev(fs);
        fs->ffs_config = NULL;
        free(fs);
        g_fusefs = NULL;
    }
    return;
}

fusefs_fuse_fs_t * get_fusefs_fuse_fs()
{
    return g_fusefs;
}


