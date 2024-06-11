#include <stdlib.h>
#include <string.h>
#include "fusefs_fuse_def.h"
#include "fusefs_fs.h"
#include "fusefs_log.h"


static fusefs_fs_t * g_fusefs = NULL;
struct fuse_args g_fusefs_fuse_args;
static struct fuse * g_fusefs_fuse = NULL;
static struct fuse_session * g_fusefs_fuse_session = NULL;


static int fusefs_mount(struct fusefs_fs * fs)
{
    FUSEFS_TRACE("fusefs_mount enter");
    fs->fusefs_storage->s_op->mount(fs->fusefs_storage);
    FUSEFS_TRACE("fusefs_mount exit");
    return 0;
}

static int fusefs_umount(struct fusefs_fs * fs)
{
    FUSEFS_TRACE("fusefs_fuse_unmount enter");
    fs->fusefs_storage->s_op->umount(fs->fusefs_storage);
    FUSEFS_TRACE("fusefs_fuse_unmount exit");
    return 0;
}

static void fusefs_get_args(struct fusefs_fs * fs, struct fuse_args ** args)
{
    fuse_opt_add_arg(&g_fusefs_fuse_args, "fusefs");

    *args = &g_fusefs_fuse_args;
    return;
}

static int fusefs_fs_init(struct fusefs_fs * fs)
{
    int rc = 0;
    struct fuse * fuse = NULL;
    struct fuse_args * args = NULL;

    FUSEFS_TRACE("fusefs_fuse_init enter");
    fusefs_get_args(fs, &args);

    fs->fusefs_fs_op = fusefs_get_fs_op();
    fuse = fuse_new(args, fs->fusefs_fs_op,
                    sizeof(struct fuse_operations), fs);
    if (fuse == NULL) {
        rc= -1;
        FUSEFS_ERROR("fuse_new errno(%d)", rc);
        goto l_free_args;
    }

    rc = fs->fusefs_storage->s_op->init(fs->fusefs_storage);
    if (rc < 0) {
        FUSEFS_ERROR("fuse_new errno(%d)", rc);
        goto l_free_fuse;
    }

    g_fusefs_fuse = fuse;
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_init ok");

l_out:
    FUSEFS_TRACE("fusefs_fuse_init exit");
   return rc;

l_free_fuse:
    fuse_destroy(fuse);
l_free_args:
    fuse_opt_free_args(args);
    goto l_out;
}

static int fusefs_fs_exit(struct fusefs_fs * fs)
{
    struct fuse * fuse = NULL;
    struct fuse_args * args = NULL;

    FUSEFS_TRACE("fusefs_fuse_exit enter");
    fuse = g_fusefs_fuse;
    args = &g_fusefs_fuse_args;

    if (fs->fusefs_storage) {
        fs->fusefs_storage->s_op->exit(fs->fusefs_storage);
    }

    fs->fusefs_fs_op = NULL;
    if (fuse) {
        fuse_destroy(fuse);
    }
    fuse_opt_free_args(args);
    FUSEFS_TRACE("fusefs_fuse_exit exit");

   return 0;
}

static int fusefs_fs_run(int argc, char *argv[], struct fusefs_fs * fs)
{
    int rc = 0;
    static struct fuse * fuse = NULL;
    struct fuse_session * session = NULL;
    char * mount_point = NULL;

    FUSEFS_TRACE("fusefs_fuse_run enter");
    mount_point = fs->fusefs_fs_config.fusefs_mountpoint;
    fuse = g_fusefs_fuse;

    fusefs_mount(fs);
    session = fuse_get_session(fuse);
    if (session == NULL) {
         FUSEFS_ERROR("fuse_get_session failed, errno(%d)", rc);
        rc = -1;
        goto l_umount;
    }

    rc = fuse_session_mount(session, mount_point);
    if (rc != 0) {
        FUSEFS_ERROR("fuse_session_mount failed, errno(%d), mount_point(%s)", rc, mount_point);
        rc = -1;
        goto l_umount;
    }

    rc= fuse_daemonize(1);
    rc = fuse_set_signal_handlers(session);
    if (rc != 0) {
        FUSEFS_ERROR("fuse_session_mount failed, errno(%d)", rc);
        rc = 1;
        goto l_session_umount;
    }
    
    rc = fuse_session_loop(session);
    //rc = fuse_session_loop_mt(fs->fusefs_session, 0);
    if (rc != 0) {
        FUSEFS_ERROR("fuse_session_loop failed, errno(%d)", rc);
        rc = -1;
        goto l_session_umount;
       }

    fuse_remove_signal_handlers(session);
    fuse_session_unmount(session);
    fusefs_umount(fs);
    FUSEFS_TRACE("fusefs_fuse_run exit");
    rc = 0;

l_out:
   return rc;

l_remove_session:
    fuse_remove_signal_handlers(session);
l_session_umount:
   fuse_session_unmount(session);
l_umount:
   fusefs_umount(fs);
    goto l_out;
}


int fusefs_malloc_fs(fusefs_config_t * config, fusefs_fs_t** fs)
{
    int rc = 0;
    int index = 0;
    fusefs_fs_t * entry = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("malloc_fusefs_fs enter");
    entry = (fusefs_fs_t*)malloc(sizeof(fusefs_fs_t));
    if (!entry) {
        goto l_out;
    }
    for (index = 0; index < config->fusefs_bdevs_num; index++) {
        entry->fusefs_fs_config.fusefs_bdevs[index] = strdup(config->fusefs_bdevs[index]);
    }
    entry->fusefs_fs_config.fusefs_bdevs_num = config->fusefs_bdevs_num;
    entry->fusefs_fs_config.fusefs_bdevs[entry->fusefs_fs_config.fusefs_bdevs_num] = NULL;
    entry->fusefs_fs_config.fusefs_mountpoint = strdup(config->fusefs_mountpoint);
    entry->fusefs_fs_config.fusefs_fsname = strdup(config->fusefs_fsname);

    entry->init = fusefs_fs_init;
    entry->exit = fusefs_fs_exit;
    entry->run = fusefs_fs_run;

    rc = fusefs_malloc_storage(config, &storage);
    if (rc < 0) {
        goto l_out;
    }
    entry->fusefs_storage = storage;
    g_fusefs = entry;
    *fs = g_fusefs;
    FUSEFS_TRACE("fusefs_malloc_fs exit");

l_out:
    return 0;
}

void fusefs_free_fs(fusefs_fs_t * fs)
{
    FUSEFS_TRACE("fusefs_free_fs enter");
    if (fs != NULL) {
        free(fs);
        g_fusefs = NULL;
    }
    FUSEFS_TRACE("free_fusefs_fs exit");
    return;
}

fusefs_fs_t * fusefs_get_fs()
{
    FUSEFS_TRACE("fusefs_get_fs enter");
    return g_fusefs;
    FUSEFS_TRACE("fusefs_get_fs exit");
}


