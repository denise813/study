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
static int passfs_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    //passfs_t * fs = (passfs_t *)(private);
    YSFS_TRACE("passfs_stat enter");

    rc = statvfs(path, s);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("statvfs failed, errno(%d)", rc);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("passfs_stat exit");

l_out:
    return rc;
}

static int passfs_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;

    YSFS_TRACE("passfs_getattr enter");
    rc = lstat(path, s);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("lstat failed, errno(%d)", rc);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("passfs_getattr exit");

l_out:
    return rc;
}

static int passfs_access(void * private, const char *path, int mask)
{
    int rc = 0;

    YSFS_TRACE("passfs_access enter");
    rc = access(path, mask);
    if (rc < 0) {
        rc = -errno;
        YSFS_ERROR("access failed, errno(%d)", rc);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("passfs_access exit");

l_out:
    return 0;
}

static int passfs_mkdir(void * private, const char *path, mode_t mode)
{
    YSFS_TRACE("passfs_mkdir enter");
    mkdir(path, mode);
    YSFS_TRACE("passfs_mkdir exit");
    return 0;
}

static int passfs_opendir(void * private, const char *path, DIR **dp)
{
    DIR *entry = NULL;

    YSFS_TRACE("passfs_mkdir enter");
    entry = opendir(path);
    *dp = entry;
    YSFS_TRACE("passfs_mkdir exit");
    return 0;
}

static int passfs_closedir(void * private, DIR *dp)
{
    YSFS_TRACE("passfs_closedir enter");
    closedir(dp);
    YSFS_TRACE("passfs_closedir exit");
    return 0;
}

static int passfs_readdir(void * private, DIR *dp,
                    struct dirent **de)
{
    struct dirent * entry = NULL;

    YSFS_TRACE("passfs_readdir enter");
    entry = readdir(dp);
    *de = entry;
    YSFS_TRACE("passfs_readdir exit");

    return 0;
}

static int passfs_rename(void * private, const char *from, const char *to)
{
    YSFS_TRACE("passfs_rename enter");
    rename(from, to);
    YSFS_TRACE("passfs_rename exit");

    return 0;
}

static int passfs_unlink(void * private, const char *path)
{
    YSFS_TRACE("passfs_unlink enter");
    unlink(path);
    YSFS_TRACE("passfs_unlink exit");
    return 0;
}

static int passfs_open(void * private, const char *path, int flags, int *fd)
{
    int entry = -1;

    YSFS_TRACE("passfs_open enter");
    entry = open(path, flags);
    YSFS_TRACE("passfs_open exit");
    *fd = entry;

    return 0;
}

static int passfs_close(void * private, int fd)
{
    YSFS_TRACE("passfs_close enter");
    close(fd);
    YSFS_TRACE("passfs_close exit");
    return 0;
}

static int passfs_read(void * private, int fd, char *buff, size_t buff_size,
                    off_t off)
{
    YSFS_TRACE("passfs_read enter");
    read(fd, buff, buff_size);
    YSFS_TRACE("passfs_read exit");
    return 0;
}

static int passfs_write(void * private,
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

static int passfs_fsync(void * private,
                int fd,
                int isdatasync)
{
    //fsync()
    YSFS_TRACE("passfs_fsync enter");
    YSFS_TRACE("passfs_fsync exit");
    return 0;
}

static int passfs_ftruncate(void * private,int fd, off_t size)
{
    YSFS_TRACE("passfs_ftruncate enter");
    ftruncate(fd, size);
    YSFS_TRACE("passfs_ftruncate exit");
    return 0;
}

static int passfs_truncate(void * private,const char *path, off_t size)
{
    YSFS_TRACE("passfs_truncate enter");
    truncate(path, size);
    YSFS_TRACE("passfs_truncate exit");
    return 0;
}

static int passfs_symlink(void * private,const char *from, const char *to)
{
    YSFS_TRACE("passfs_symlink enter");
    symlink(from, to);
    YSFS_TRACE("passfs_symlink enter");
    return 0;
}
    // unsupported functions
static int passfs_link(void * private,const char *from, const char *to)
{
    YSFS_TRACE("passfs_link enter");
    link(from, to);
    YSFS_TRACE("passfs_link exit");
    return 0;
}

static int passfs_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    YSFS_TRACE("passfs_mknod enter");
    mknod(path, mode, dev);
    YSFS_TRACE("passfs_mknod exit");
    return 0;
}

static int passfs_chmod(void * private,const char *path, mode_t mode)
{
    YSFS_TRACE("passfs_truncate enter");
    chmod(path, mode);
    YSFS_TRACE("passfs_truncate exit");
    return 0;
}

static int passfs_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    YSFS_TRACE("passfs_chown enter");
    YSFS_TRACE("passfs_chown exit");
    return -1;
}

static int passfs_mount(void * private)
{
    YSFS_TRACE("passfs_mount enter");
    YSFS_TRACE("passfs_mount exit");
    return 0;
}

static int passfs_unmount(void * private)
{
     YSFS_TRACE("passfs_unmount enter");
     YSFS_TRACE("passfs_unmount exit");
    return 0;
}

static vfs_syscall_t g_passfs_syscall_op =
{
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
    .mount = passfs_mount,
    .unmount = passfs_unmount
};


int passfs_malloc_fs(passfs_t **fs)
{
    passfs_t * entry = NULL;

    YSFS_TRACE("malloc_passfs enter");
    entry = malloc(sizeof(passfs_t));
    //entry->passfs_block;
    entry->vfs_op = &g_passfs_syscall_op;
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

