#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "src/fs/vfs_op.h"
#include "src/ysfs_log.h"
#include "passfs.h"


/* --comment by louting, 2024/5/13--
 * 
 */
#define PASS_PATH_MAX 1024

static int passfs_trans_path(void * private, const char * path, char * real_path)
{
    passfs_t * fs = (passfs_t*)private;
    passfs_config_t * config = &fs->pfs_config;

    snprintf(real_path, PASS_PATH_MAX, "%s/%s", config->pfs_bdev, path);
    return 0;
}

static int passfs_infs(void * private, const char * path)
{
    YSFS_TRACE("passfs_infs enter %s", path);
    YSFS_TRACE("passfs_infs exit %s", path);
    return 0;
}

static int passfs_syscall_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_stat enter %s-%s", path, real_path);

    rc = statvfs(real_path, s);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("statvfs failed, errno(%d) %s-%s", rc, path, real_path);
        rc = 0;
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_stat ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_stat exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_getattr enter %s-%s", path, real_path);
    rc = lstat(real_path, s);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("lstat failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_getattr ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_getattr exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_access(void * private, const char *path, int mask)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_access enter %s-%s", path, real_path);

    rc = access(real_path, mask);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("passfs_access ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_access exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_mkdir enter %s-%s", path, real_path);

    rc = mkdir(real_path, mode);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_mkdir ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_mkdir exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    DIR * passfs_dp = NULL;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_opendir enter %s-%s", path, real_path);

    passfs_dp = opendir(real_path);
    if (!passfs_dp) {
        rc = -errno;
        YSFS_ERROR("opendir failed %d %s-%s", rc, path, real_path);
        rc = -1;
        goto l_out;
    }

    rc = 0;
    *dp = (void*)passfs_dp;
    YSFS_TRACE("passfs_opendir exit %s-%s %p", path, real_path, *dp);

l_out:
    YSFS_TRACE("passfs_opendir exit %s-%s %p", path, real_path);
    return rc;
}

static int passfs_syscall_closedir(void * private, void *dp)
{
    int rc = 0;
    DIR * passfs_dp = dp;

    YSFS_TRACE("passfs_closedir enter %p", dp);
    rc = closedir(passfs_dp);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("closedir failed %d %p", rc, dp);
        rc = -1;
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_closedir ok %p", dp);

l_out:
    YSFS_TRACE("passfs_closedir exit %p", dp);
    return 0;
}

static int passfs_syscall_readdir(void * private, void *dp,
                    struct dirent **de)
{
    int rc = 0;
     DIR *passfs_dp = NULL;
    struct dirent * entry = NULL;

    YSFS_TRACE("passfs_readdir enter %p", dp);
    passfs_dp = (DIR*)dp;
    entry = readdir(passfs_dp);
    if (!entry) {
        rc = -errno;
        YSFS_ERROR("readdir failed %d", rc);
        rc = -1;
        goto l_out;
    }

    *de = entry;
    rc = 0;
    YSFS_TRACE("passfs_readdir ok %p", dp);

l_out:
    YSFS_TRACE("passfs_readdir exit %p", dp);
    return rc;
}

static int passfs_syscall_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
    char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, to, passfs_path_from);
    passfs_trans_path(private, to, passfs_path_to);
    YSFS_TRACE("passfs_rename enter %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

    rc = rename(passfs_path_from, passfs_path_to);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("rename failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        passfs_path_from,
                        passfs_path_to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_rename ok %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

l_out:
    YSFS_TRACE("passfs_rename exit %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);
    return rc;
}

static int passfs_syscall_unlink(void * private, const char *path)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_unlink enter. %s-%s", path, real_path);
 
    rc = unlink(real_path);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("unlink failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_unlink ok. %s-%s", path, real_path);

l_out:
     YSFS_TRACE("passfs_unlink exit. %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;
    int *passfs_file = NULL;;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_open enter %s-%s", path, real_path);

    passfs_file = (int *)malloc(sizeof(int));
    if (!passfs_file) {
        rc = -ENOMEM;
        YSFS_ERROR("malloc failed %d %s-%s", rc, path, real_path);
         goto l_out;
    }

    *passfs_file = open(real_path, flags);
    if (*passfs_file < 0) {
        rc = -errno;
        YSFS_ERROR("open failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    *file = (void*)passfs_file;
    YSFS_TRACE("passfs_open ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_open exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_close(void * private, void *file)
{
    int rc = 0;
    int *passfs_file = NULL;

    YSFS_TRACE("passfs_close enter");

    passfs_file = (int*)(file);
    rc = close(*passfs_file);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("close failed %d", rc);
        goto l_out;
    }

     rc = 0;
    YSFS_TRACE("passfs_close exit");

l_out:
    YSFS_TRACE("passfs_close exit");
    return rc;
}

static ssize_t passfs_syscall_read(void * private, void * file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    int *passfs_file = NULL;
    ssize_t size = 0;

    YSFS_TRACE("passfs_read enter");

    passfs_file = (int*)(file);
    size = read(*passfs_file, buff, buff_size);
    if (size < 0) {
        rc = -errno;
        size  = rc;
        YSFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_read ok");

l_out:
    YSFS_TRACE("passfs_read exit");
    return size;
}

static ssize_t passfs_syscall_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    int *passfs_file = NULL;
    ssize_t size = 0;

    YSFS_TRACE("passfs_write enter");

    passfs_file = (int*)(file);
    size = write(*passfs_file, buff, buff_size);
    if (size < 0) {
        rc = -errno;
        size  = rc;
        YSFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_write ok");

l_out:
    YSFS_TRACE("passfs_write exit");
    return size;
}

static int passfs_syscall_fsync(void * private,
                void * file,
                int isdatasync)
{
    //fsync()
    YSFS_TRACE("passfs_fsync enter");
    YSFS_TRACE("passfs_fsync exit");
    return 0;
}

static int passfs_syscall_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    int *passfs_file = NULL;

    YSFS_TRACE("passfs_ftruncate enter");

    passfs_file = (int*)(file);
    rc = ftruncate(*passfs_file, size);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_ftruncate ok");

l_out:
    YSFS_TRACE("passfs_ftruncate exit");
    return rc;
}

static int passfs_syscall_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
   char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_truncate enter %s-%s", path, real_path);

    rc = truncate(real_path, size);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("truncate failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_truncate ok %s-%s", path, real_path);

l_out:
     YSFS_TRACE("passfs_truncate exit %s-%s", path, real_path);
    return 0;
}

static int passfs_syscall_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
     char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, from, passfs_path_from);
    passfs_trans_path(private, to, passfs_path_to);
    YSFS_TRACE("passfs_symlink enter %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

    rc = symlink(passfs_path_from, passfs_path_to);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("symlink failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        passfs_path_from,
                        passfs_path_to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_symlink ok %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

l_out:
    YSFS_TRACE("passfs_symlink exit %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);
    return rc;
}
    // unsupported functions
static int passfs_syscall_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
    char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, from, real_path_from);
    passfs_trans_path(private, to, real_path_to);
    YSFS_TRACE("passfs_syscall_link enter %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

    rc = link(real_path_from, real_path_to);
    if (rc < 0) {
        rc = -errno;
        YSFS_TRACE("link failed errno(%d) %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        passfs_path_from,
                        passfs_path_to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_syscall_link ok %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);

l_out:
    YSFS_TRACE("passfs_syscall_link exit %s-%s, %s-%s",
                    from,
                    to,
                    passfs_path_from,
                    passfs_path_to);
    return rc;
}

static int passfs_syscall_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_mknod enter %s-%s", path, real_path);

    rc = mknod(real_path, mode, dev);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("passfs_mknod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_mknod ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_mknod exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_truncate enter %s-%s", path, real_path);

    rc = chmod(real_path, mode);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("passfs_truncate failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_truncate ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("passfs_truncate exit %s-%s", path, real_path);
    return rc;
}

static int passfs_syscall_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_chown enter %s-%s", path, real_path);

    rc = chown(path, uid, gid);
     if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("passfs_chown failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_chown ok %s-%s", path, real_path);

l_out:
     YSFS_TRACE("passfs_chown exit %s-%s", path, real_path);
    return rc;
}

static int passfs_mount(void * private)
{
    passfs_t * fs = (passfs_t *)(private);
    YSFS_TRACE("passfs_mount enter ");
    YSFS_TRACE("passfs_mount exit");
    return 0;
}

static int passfs_unmount(void * private)
{
    passfs_t * fs = (passfs_t *)(private);
    YSFS_TRACE("passfs_unmount enter");

    YSFS_TRACE("passfs_unmount exit");
    return 0;
}

static vfs_syscall_t g_passfs_syscall_op =
{
    .stat = passfs_syscall_stat,
    .getattr = passfs_syscall_getattr,
    .access = passfs_syscall_access,
    .mkdir = passfs_syscall_mkdir,
    .opendir = passfs_syscall_opendir,
    .closedir = passfs_syscall_closedir,
    .readdir = passfs_syscall_readdir,
    .rename = passfs_syscall_rename,
    .unlink = passfs_syscall_unlink,
    .open = passfs_syscall_open,
    .close = passfs_syscall_close,
    .read = passfs_syscall_read,
    .write = passfs_syscall_write,
    .fsync = passfs_syscall_fsync,
    .ftruncate = passfs_syscall_ftruncate,
    .truncate = passfs_syscall_truncate,
    .link = passfs_syscall_link,
    .mknod = passfs_syscall_mknod,
    .chmod = passfs_syscall_chmod,
    .chown= passfs_syscall_chown,
    .mount = passfs_mount,
    .unmount = passfs_unmount,
    .infs = passfs_infs,
};


int passfs_malloc_fs(passfs_t **fs)
{
    passfs_t * entry = NULL;

    YSFS_TRACE("malloc_passfs enter");
    entry = malloc(sizeof(passfs_t));
    //entry->passfs_block;
    entry->pfs_op = &g_passfs_syscall_op;
    
    *fs = entry;
    YSFS_TRACE("malloc_passfs exit");
    return 0;
}

void passfs_free_fs(passfs_t *fs)
{
    YSFS_TRACE("free_passfs enter");
    free(fs);
     YSFS_TRACE("free_passfs exit");
}

