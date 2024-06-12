#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "../../fusefs_storage_op.h"
#include "../../fusefs_storage.h"
#include "../../fusefs_log.h"
#include "passfs_inter.h"


/* --comment by louting, 2024/5/13--
 * 
 */
#define PASS_PATH_MAX 1024

static int passfs_trans_path(void * private, const char * path, char * real_path)
{
    passfs_t * fs = (passfs_t*)private;
    passfs_config_t * config = &fs->pfs_config;

    snprintf(real_path, PASS_PATH_MAX, "%s/%s", config->pfs_root, path);
    return 0;
}


static int passfs_init(void * private)
{
    FUSEFS_TRACE("passfs_init enter");
    FUSEFS_TRACE("passfs_init exit");
    return 0;
}

static int passfs_exit(void * private)
{
    FUSEFS_TRACE("passfs_exit enter");
    FUSEFS_TRACE("passfs_exit exit");
    return 0;
}

static int passfs_mount(void * private)
{
    passfs_t * fs = (passfs_t *)(private);
    FUSEFS_TRACE("passfs_mount enter ");
    FUSEFS_TRACE("passfs_mount exit");
    return 0;
}

static int passfs_umount(void * private)
{
    passfs_t * fs = (passfs_t *)(private);
    FUSEFS_TRACE("passfs_unmount enter");

    FUSEFS_TRACE("passfs_unmount exit");
    return 0;
}

static int passfs_infs(void * private, const char * path)
{
    FUSEFS_TRACE("passfs_infs enter %s", path);
    FUSEFS_TRACE("passfs_infs exit %s", path);
    return 0;
}

static int passfs_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_stat enter %s-%s", path, real_path);

    rc = statvfs(real_path, s);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("statvfs failed, errno(%d) %s-%s", rc, path, real_path);
        rc = 0;
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_stat ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_stat exit %s-%s", path, real_path);
    return rc;
}

static int passfs_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_getattr enter %s-%s", path, real_path);
    rc = lstat(real_path, s);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("lstat failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_getattr ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_getattr exit %s-%s", path, real_path);
    return rc;
}

static int passfs_access(void * private, const char *path, int mask)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_access enter %s-%s", path, real_path);

    rc = access(real_path, mask);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("passfs_access ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_access exit %s-%s", path, real_path);
    return rc;
}

static int passfs_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_mkdir enter %s-%s", path, real_path);

    rc = mkdir(real_path, mode);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_mkdir ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_mkdir exit %s-%s", path, real_path);
    return rc;
}

static int passfs_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    DIR * passfs_dp = NULL;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_opendir enter %s-%s", path, real_path);

    passfs_dp = opendir(real_path);
    if (!passfs_dp) {
        rc = -errno;
        FUSEFS_ERROR("opendir failed %d %s-%s", rc, path, real_path);
        rc = -1;
        goto l_out;
    }

    rc = 0;
    *dp = (void*)passfs_dp;
    FUSEFS_TRACE("passfs_opendir exit %s-%s %p", path, real_path, *dp);

l_out:
    FUSEFS_TRACE("passfs_opendir exit %s-%s %p", path, real_path);
    return rc;
}

static int passfs_closedir(void * private, void *dp)
{
    int rc = 0;
    DIR * passfs_dp = dp;

    FUSEFS_TRACE("passfs_closedir enter %p", dp);
    rc = closedir(passfs_dp);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("closedir failed %d %p", rc, dp);
        rc = -1;
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_closedir ok %p", dp);

l_out:
    FUSEFS_TRACE("passfs_closedir exit %p", dp);
    return 0;
}

static int passfs_readdir(void * private, void *dp,
                    struct dirent **de)
{
    int rc = 0;
     DIR *passfs_dp = NULL;
    struct dirent * entry = NULL;

    FUSEFS_TRACE("passfs_readdir enter %p", dp);
    passfs_dp = (DIR*)dp;
    entry = readdir(passfs_dp);
    if (!entry) {
        rc = -errno;
        FUSEFS_ERROR("readdir failed %d", rc);
        rc = -1;
        goto l_out;
    }

    *de = entry;
    rc = 0;
    FUSEFS_TRACE("passfs_readdir ok %p", dp);

l_out:
    FUSEFS_TRACE("passfs_readdir exit %p", dp);
    return rc;
}

static int passfs_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
    char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, to, passfs_path_from);
    passfs_trans_path(private, to, passfs_path_to);
    FUSEFS_TRACE("passfs_rename enter %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

    rc = rename(passfs_path_from, passfs_path_to);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("rename failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        passfs_path_from,
                        passfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_rename ok %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

l_out:
    FUSEFS_TRACE("passfs_rename exit %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);
    return rc;
}

static int passfs_unlink(void * private, const char *path)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_unlink enter. %s-%s", path, real_path);
 
    rc = unlink(real_path);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("unlink failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_unlink ok. %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("passfs_unlink exit. %s-%s", path, real_path);
    return rc;
}

static int passfs_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;
    int *passfs_file = NULL;;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_open enter %s-%s", path, real_path);

    passfs_file = (int *)malloc(sizeof(int));
    if (!passfs_file) {
        rc = -ENOMEM;
        FUSEFS_ERROR("malloc failed %d %s-%s", rc, path, real_path);
         goto l_out;
    }

    *passfs_file = open(real_path, flags);
    if (*passfs_file < 0) {
        rc = -errno;
        FUSEFS_ERROR("open failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    *file = (void*)passfs_file;
    FUSEFS_TRACE("passfs_open ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_open exit %s-%s", path, real_path);
    return rc;
}

static int passfs_close(void * private, void *file)
{
    int rc = 0;
    int *passfs_file = NULL;

    FUSEFS_TRACE("passfs_close enter");

    passfs_file = (int*)(file);
    rc = close(*passfs_file);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("close failed %d", rc);
        goto l_out;
    }

     rc = 0;
    FUSEFS_TRACE("passfs_close exit");

l_out:
    FUSEFS_TRACE("passfs_close exit");
    return rc;
}

static ssize_t passfs_read(void * private, void * file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    int *passfs_file = NULL;
    ssize_t size = 0;

    FUSEFS_TRACE("passfs_read enter");

    passfs_file = (int*)(file);
    size = read(*passfs_file, buff, buff_size);
    if (size < 0) {
        rc = -errno;
        size  = rc;
        FUSEFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_read ok");

l_out:
    FUSEFS_TRACE("passfs_read exit");
    return size;
}

static ssize_t passfs_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    int *passfs_file = NULL;
    ssize_t size = 0;

    FUSEFS_TRACE("passfs_write enter");

    passfs_file = (int*)(file);
    size = write(*passfs_file, buff, buff_size);
    if (size < 0) {
        rc = -errno;
        size  = rc;
        FUSEFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_write ok");

l_out:
    FUSEFS_TRACE("passfs_write exit");
    return size;
}

static int passfs_fsync(void * private,
                void * file,
                int isdatasync)
{
    //fsync()
    FUSEFS_TRACE("passfs_fsync enter");
    FUSEFS_TRACE("passfs_fsync exit");
    return 0;
}

static int passfs_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    int *passfs_file = NULL;

    FUSEFS_TRACE("passfs_ftruncate enter");

    passfs_file = (int*)(file);
    rc = ftruncate(*passfs_file, size);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_ftruncate ok");

l_out:
    FUSEFS_TRACE("passfs_ftruncate exit");
    return rc;
}

static int passfs_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
   char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_truncate enter %s-%s", path, real_path);

    rc = truncate(real_path, size);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("truncate failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_truncate ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("passfs_truncate exit %s-%s", path, real_path);
    return 0;
}

static int passfs_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
     char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, from, passfs_path_from);
    passfs_trans_path(private, to, passfs_path_to);
    FUSEFS_TRACE("passfs_symlink enter %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

    rc = symlink(passfs_path_from, passfs_path_to);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("symlink failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        passfs_path_from,
                        passfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_symlink ok %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

l_out:
    FUSEFS_TRACE("passfs_symlink exit %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);
    return rc;
}
    // unsupported functions
static int passfs_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
    char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, from, real_path_from);
    passfs_trans_path(private, to, real_path_to);
    FUSEFS_TRACE("passfs_link enter %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

    rc = link(real_path_from, real_path_to);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_TRACE("link failed errno(%d) %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        passfs_path_from,
                        passfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_link ok %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

l_out:
    FUSEFS_TRACE("passfs_link exit %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);
    return rc;
}

static int passfs_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_mknod enter %s-%s", path, real_path);

    rc = mknod(real_path, mode, dev);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("passfs_mknod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_mknod ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_mknod exit %s-%s", path, real_path);
    return rc;
}

static int passfs_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_chmod enter %s-%s", path, real_path);

    rc = chmod(real_path, mode);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("chmod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_chmod ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("passfs_chmod exit %s-%s", path, real_path);
    return rc;
}

static int passfs_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("passfs_chown enter %s-%s", path, real_path);

    rc = chown(path, uid, gid);
     if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("passfs_chown failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_chown ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("passfs_chown exit %s-%s", path, real_path);
    return rc;
}

static storage_op_t g_passfs_op =
{
    .init = passfs_init,
    .exit = passfs_exit,
    .mount = passfs_mount,
    .umount = passfs_umount,
    .stat = passfs_stat,
    .getattr = passfs_getattr,
    .access = passfs_access,
    .mkdir = passfs_mkdir,
    .opendir = passfs_opendir,
    .closedir = passfs_closedir,
    .readdir = passfs_readdir,
    .rename = passfs_rename,
    .unlink = passfs_unlink,
    .open = passfs_open,
    .close = passfs_close,
    .read = passfs_read,
    .write = passfs_write,
    .fsync = passfs_fsync,
    .ftruncate = passfs_ftruncate,
    .truncate = passfs_truncate,
    .link = passfs_link,
    .mknod = passfs_mknod,
    .chmod = passfs_chmod,
    .chown= passfs_chown,
    .infs = passfs_infs,
};


int passfs_malloc_fs(void * fuse_storage)
{
    int rc = 0;
    passfs_t * entry = NULL;
    char *dev = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("malloc_passfs enter");
    storage = (fusefs_storage_t*)(fuse_storage);
    entry = malloc(sizeof(passfs_t));
    if (!entry){
        rc = -ENOMEM;
        goto l_out;
    }
    //entry->passfs_block;
    entry->pfs_op = &g_passfs_op;
    dev = (char*)storage->s_config.sc_devs[0];
    entry->pfs_config.pfs_bdev = strdup(dev);
    entry->pfs_config.pfs_root = entry->pfs_config.pfs_bdev;
    
    storage->s_agent.as_stroage = entry;
    storage->s_agent.as_stroage_op = &g_passfs_op;

    FUSEFS_TRACE("malloc_passfs exit");

l_out:
    return 0;
}

void passfs_free_fs(void *fuse_storage)
{
    fusefs_storage_t * storage = (fusefs_storage_t*)(fuse_storage);
    passfs_t * entry = NULL;
    
    FUSEFS_TRACE("free_passfs enter");
    entry->pfs_config.pfs_root = NULL;
 
    free(entry->pfs_config.pfs_bdev);
    free(storage->s_agent.as_stroage);
    storage->s_agent.as_stroage = NULL;
    storage->s_agent.as_stroage_op = NULL;
     FUSEFS_TRACE("free_passfs exit");
}

