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
        YSFS_ERROR("statvfs failed, errno(%d)", rc);
        rc = 0;
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("passfs_stat exit");

l_out:
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
        rc = -1;
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("passfs_getattr exit %s", path);

l_out:
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
    YSFS_TRACE("passfs_access exit %s-%s", path, real_path);

l_out:
    return 0;
}

static int passfs_syscall_mkdir(void * private, const char *path, mode_t mode)
{
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_mkdir enter %s-%s", path, real_path);
    mkdir(real_path, mode);
    YSFS_TRACE("passfs_mkdir exit %s-%s", path, real_path);
    return 0;
}

static int passfs_syscall_opendir(void * private, const char *path, DIR **dp)
{
    int rc = 0;
    DIR *entry = NULL;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_mkdir enter %s-%s", path, real_path);
    entry = opendir(real_path);
    if (!entry) {
        rc = -errno;
        YSFS_ERROR("passfs_readdir failed %d %s-%s", rc, path, real_path);
        rc = -1;
        goto l_out;
    }
    *dp = entry;
    rc = 0;
    YSFS_TRACE("passfs_mkdir exit %s-%s %p", path, real_path, *dp);
l_out:
    return 0;
}

static int passfs_syscall_closedir(void * private, DIR *dp)
{
    int rc = 0;
    YSFS_TRACE("passfs_closedir enter %p", dp);
    closedir(dp);
    YSFS_TRACE("passfs_closedir exit %p", dp);
    return 0;
}

static int passfs_syscall_readdir(void * private, DIR *dp,
                    struct dirent **de)
{
    int rc = 0;
    struct dirent * entry = NULL;

    YSFS_TRACE("passfs_readdir enter %p", dp);
    entry = readdir(dp);
    if (!entry) {
        rc = -errno;
        YSFS_ERROR("passfs_readdir failed %d", rc);
        rc = -1;
        goto l_out;
    }
    *de = entry;
    rc = 0;
    YSFS_TRACE("passfs_readdir exit %p", dp);
l_out:
    return rc;
}

static int passfs_syscall_rename(void * private, const char *from, const char *to)
{
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
    char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, to, passfs_path_from);
    passfs_trans_path(private, to, passfs_path_to);
    YSFS_TRACE("passfs_rename enter %s-%s-%s", from, to, passfs_path_to);
    rename(passfs_path_from, passfs_path_to);
    YSFS_TRACE("passfs_rename exit");

    return 0;
}

static int passfs_syscall_unlink(void * private, const char *path)
{
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_unlink enter. %s-%s", path, real_path);
    unlink(real_path);
    YSFS_TRACE("passfs_unlink exit");
    return 0;
}

static int passfs_syscall_open(void * private, const char *path, int flags, int *fd)
{
    int entry = -1;
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_open enter %s-%s", path, real_path);
    entry = open(real_path, flags);
    YSFS_TRACE("passfs_open exit");
    *fd = entry;

    return 0;
}

static int passfs_syscall_close(void * private, int fd)
{
    YSFS_TRACE("passfs_close enter");
    close(fd);
    YSFS_TRACE("passfs_close exit");
    return 0;
}

static int passfs_syscall_read(void * private, int fd, char *buff, size_t buff_size,
                    off_t off)
{
    YSFS_TRACE("passfs_read enter");
    read(fd, buff, buff_size);
    YSFS_TRACE("passfs_read exit");
    return 0;
}

static int passfs_syscall_write(void * private,
                int fd,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    YSFS_TRACE("passfs_write enter");
    write(fd, buff, buff_size);
    YSFS_TRACE("passfs_write exit");
    return 0;
}

static int passfs_syscall_fsync(void * private,
                int fd,
                int isdatasync)
{
    //fsync()
    YSFS_TRACE("passfs_fsync enter");
    YSFS_TRACE("passfs_fsync exit");
    return 0;
}

static int passfs_syscall_ftruncate(void * private,int fd, off_t size)
{
    YSFS_TRACE("passfs_ftruncate enter");
    ftruncate(fd, size);
    YSFS_TRACE("passfs_ftruncate exit");
    return 0;
}

static int passfs_syscall_truncate(void * private,const char *path, off_t size)
{
    char passfs_path[PASS_PATH_MAX] = {0};
   char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_truncate enter %s-%s", path, real_path);
    truncate(real_path, size);
    YSFS_TRACE("passfs_truncate exit");
    return 0;
}

static int passfs_syscall_symlink(void * private,const char *from, const char *to)
{
    char passfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = passfs_path_from;
     char passfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = passfs_path_to;

    passfs_trans_path(private, from, passfs_path_from);
    passfs_trans_path(private, to, passfs_path_to);
    YSFS_TRACE("passfs_symlink enter %s-%s-%s", from, to, passfs_path_to);
    symlink(passfs_path_from, passfs_path_to);
    YSFS_TRACE("passfs_symlink enter");
    return 0;
}
    // unsupported functions
static int passfs_syscall_link(void * private,const char *from, const char *to)
{
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, to, real_path);
    YSFS_TRACE("passfs_link enter");
    link(from, real_path);
    YSFS_TRACE("passfs_link exit");
    return 0;
}

static int passfs_syscall_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_mknod enter %s-%s", path, real_path);
    mknod(real_path, mode, dev);
    YSFS_TRACE("passfs_mknod exit");
    return 0;
}

static int passfs_syscall_chmod(void * private,const char *path, mode_t mode)
{
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_truncate enter %s-%s", path, real_path);
    chmod(real_path, mode);
    YSFS_TRACE("passfs_truncate exit");
    return 0;
}

static int passfs_syscall_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    char passfs_path[PASS_PATH_MAX] = {0};
    char * real_path = passfs_path;

    passfs_trans_path(private, path, real_path);
    YSFS_TRACE("passfs_chown enter %s-%s", path, real_path);
    YSFS_TRACE("passfs_chown exit");
    return -1;
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
    .unmount = passfs_unmount
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

