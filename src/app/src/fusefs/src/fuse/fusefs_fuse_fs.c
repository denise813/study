#include <stdlib.h>
#include <string.h>
#include "fusefs_fuse_def.h"
#include "fusefs_fuse.h"
#include "src/ysfs_log.h"


static fusefs_fuse_fs_t * g_fusefs = NULL;
struct fuse_args g_fusefs_fuse_args;
static struct fuse * g_fusefs_fuse = NULL;
static struct fuse_session * g_fusefs_fuse_session = NULL;


static int fusefs_register_bdev(struct fusefs_fuse_fs * fs)
{
    int rc = 0;
    fusefs_fuse_blockdev_t * bdev = NULL;
    fusefs_fuse_blockdev_config_t * cbdev = NULL;

    YSFS_TRACE("fusefs_fuse_register_bdev enter");
    cbdev = (fusefs_fuse_blockdev_config_t*)malloc(sizeof(fusefs_fuse_blockdev_config_t));
    cbdev->fb_bdev = strdup(fs->ffs_config->ffs_bdev);
    
    fusedev_malloc_blockdev(cbdev, &bdev);

    fs->ffs_bdev = bdev;
    //fs->ffs_vfs->vfs_block = fs->ffs_bdev;
    YSFS_TRACE("fusefs_fuse_register_bdev exit");
    return 0;
}

static int fusefs_unregister_bdev(struct fusefs_fuse_fs * fs)
{
    int rc = 0;
    YSFS_TRACE("fusefs_fuse_unregister_bdev enter");
    if (fs->ffs_bdev ==NULL) {
        goto l_out;
    }
    if (fs->ffs_bdev->fbdev_config->fb_bdev) {
        free(fs->ffs_bdev->fbdev_config->fb_bdev);
    }
    if (fs->ffs_bdev->fbdev_config->fb_bdev) {
        free(fs->ffs_bdev->fbdev_config);
    }
    fusedev_free_blockdev(fs->ffs_bdev);
    YSFS_TRACE("fusefs_fuse_unregister_bdev exit");
    rc = 0;
l_out:
    return rc;
}

static int fusefs_mount(struct fusefs_fuse_fs * fs)
{
    YSFS_TRACE("fusefs_fuse_mount enter");
    fs->ffs_vfs->vfs_op->mount(fs->ffs_vfs);

    //return fusefs_mount(&fusefs, &config);
    YSFS_TRACE("fusefs_fuse_mount exit");
    return 0;
}

static int fusefs_unmount(struct fusefs_fuse_fs * fs)
{
    YSFS_TRACE("fusefs_fuse_unmount enter");
    fs->ffs_vfs->vfs_op->unmount(fs->ffs_vfs);
    YSFS_TRACE("fusefs_fuse_unmount exit");
    return 0;
}

static void fusefs_get_args(struct fusefs_fuse_fs * fs, struct fuse_args ** args)
{
    fuse_opt_add_arg(&g_fusefs_fuse_args, "fusefs");
    *args = &g_fusefs_fuse_args;
    return;
}

static int fusefs_fuse_init(struct fusefs_fuse_fs * fs)
{
    int rc = 0;
    struct fuse_operations * fusefs_fuse_op = NULL;
    struct fuse * fuse = NULL;
    struct fuse_args * args = NULL;

    YSFS_TRACE("fusefs_fuse_init enter");
    fusefs_get_args(fs, &args);

    fusefs_fuse_op = fusefs_get_fs_op();

    fuse = fuse_new(args, fusefs_fuse_op,
                    sizeof(struct fuse_operations), fs);
    if (fuse == NULL) {
        rc= -1;
        YSFS_ERROR("fuse_new errno(%d)", rc);
        goto l_out;
    }
    fs->ffs_op = fusefs_fuse_op;
    fusefs_register_bdev(fs);
    vfs_config_t vfs_config;
    vfs_config.vfs_fs_type = 1;
    vfs_config.vfs_dev = fs->ffs_config->ffs_bdev;
    vfs_malloc_fs(&vfs_config, &fs->ffs_vfs);

    g_fusefs_fuse = fuse;
    rc = 0;
    YSFS_TRACE("fusefs_fuse_init exit");

l_out:
   return 0;
}

static int fusefs_fuse_exit(struct fusefs_fuse_fs * fs)
{
    struct fuse * fuse = NULL;
    struct fuse_args * args = NULL;

    YSFS_TRACE("fusefs_fuse_exit enter");
    fuse = g_fusefs_fuse;
    args = &g_fusefs_fuse_args;

    vfs_free_fs(fs->ffs_vfs);
    fusefs_unregister_bdev(fs);
    fs->ffs_op = NULL;
    fuse_destroy(fuse);
    fuse_opt_free_args(args);
    YSFS_TRACE("fusefs_fuse_exit exit");

   return 0;
}

static int fusefs_fuse_run(int argc, char *argv[], struct fusefs_fuse_fs * fs)
{
    int rc = 0;
    static struct fuse * fuse = NULL;
    struct fuse_session * session = NULL;
    char * mount_point = NULL;

    YSFS_TRACE("fusefs_fuse_run enter");
    mount_point = fs->ffs_config->ffs_mountpoint;
    fuse = g_fusefs_fuse;

    fusefs_mount(fs);
    session = fuse_get_session(fuse);
    rc = fuse_session_mount(session, mount_point);
    if (rc != 0) {
        YSFS_ERROR("fuse_session_mount failed, errno(%d)", rc);
        rc = -1;
        goto l_umount;
    }

    rc= fuse_daemonize(1);
    rc = fuse_set_signal_handlers(session);
    if (rc != 0) {
        YSFS_ERROR("fuse_session_mount failed, errno(%d)", rc);
        rc = 1;
        goto l_session_umount;
    }
    
    rc = fuse_session_loop(session);
    //rc = fuse_session_loop_mt(fs->ffs_session, 0);
    if (rc != 0) {
        YSFS_ERROR("fuse_session_loop failed, errno(%d)", rc);
        rc = -1;
        goto l_session_umount;
       }

    fuse_remove_signal_handlers(session);
    fuse_session_unmount(session);
    fusefs_unmount(fs);
    YSFS_TRACE("fusefs_fuse_run exit");
    rc = 0;

l_out:
   return rc;

l_remove_session:
    fuse_remove_signal_handlers(session);
l_session_umount:
   fuse_session_unmount(session);
l_umount:
   fusefs_unmount(fs);
    goto l_out;
}


int fusefs_malloc_fs(fusefs_fuse_fs_config_t * config, fusefs_fuse_fs_t** fs)
{
    fusefs_fuse_fs_t * entry = NULL;
    vfs_t * vfs = NULL;

    YSFS_TRACE("malloc_fusefs_fuse_fs enter");
    entry = (fusefs_fuse_fs_t*)malloc(sizeof(fusefs_fuse_fs_t));
    entry->ffs_config = config;
    //entry->ysffs_op = &g_fusefs_fuse_op;
    fusefs_register_bdev(entry);
    //entry->mount = fusefs_fuse_mount;
    //entry->umount = fusefs_fuse_unmount;
    entry->init = fusefs_fuse_init;
    entry->exit = fusefs_fuse_exit;
    entry->run = fusefs_fuse_run;
    entry->ffs_vfs = vfs;
    g_fusefs = entry;
    *fs = g_fusefs;
    YSFS_TRACE("malloc_fusefs_fuse_fs exit");
    return 0;
}

void fusefs_free_fs(fusefs_fuse_fs_t * fs)
{
    YSFS_TRACE("free_fusefs_fuse_fs enter");
    if (fs != NULL) {
        fusefs_unregister_bdev(fs);
        fs->ffs_config = NULL;
        free(fs);
        g_fusefs = NULL;
    }
    YSFS_TRACE("free_fusefs_fuse_fs exit");
    return;
}

fusefs_fuse_fs_t * fusefs_get_fs()
{
    YSFS_TRACE("get_fusefs_fuse_fs enter");
    return g_fusefs;
    YSFS_TRACE("get_fusefs_fuse_fs exit");
}


