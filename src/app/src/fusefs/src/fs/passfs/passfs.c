#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "src/fs/vfs_op.h"
#include "passfs.h"


/* --comment by louting, 2024/5/13--
 * 
 */
static int passfs_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    //passfs_t * fs = (passfs_t *)(private);

    rc = statvfs(path, s);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;
    
l_out:
    return rc;
}

static int passfs_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    rc = lstat(path, s);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
l_out:
    return rc;
}

static int passfs_access(void * private, const char *path, int mask)
{
    int rc = 0;
    rc = access(path, mask);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return 0;
}

static int passfs_mkdir(void * private, const char *path, mode_t mode)
{
    mkdir(path, mode);
    return 0;
}

static int passfs_opendir(void * private, const char *path, DIR **dp)
{
    DIR *entry = NULL;
     entry = opendir(path);
     *dp = entry;
    return 0;
}

static int passfs_closedir(void * private, DIR *dp)
{
    closedir(dp);
    return 0;
}

static int passfs_readdir(void * private, DIR *dp,
                    struct dirent **de)
{
    struct dirent * entry = NULL;
    entry = readdir(dp);
    *de = entry;
    return 0;
}

static int passfs_rename(void * private, const char *from, const char *to)
{
    rename(from, to);
    return 0;
}

static int passfs_unlink(void * private, const char *path)
{
    unlink(path);
    return 0;
}

static int passfs_open(void * private, const char *path, int flags, int *fd)
{
    int entry = -1;
    entry = open(path, flags);
    *fd = entry;
 
    return 0;
}

static int passfs_close(void * private, int fd)
{
    close(fd);
    return 0;
}

static int passfs_read(void * private, int fd, char *buff, size_t buff_size,
                    off_t off)
{
    read(fd, buff, buff_size);
    return 0;
}

static int passfs_write(void * private,
                int fd,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    write(fd, buff, buff_size);
    return 0;
}

static int passfs_fsync(void * private,
                int fd,
                int isdatasync)
{
    //fsync()
    return 0;
}

static int passfs_ftruncate(void * private,int fd, off_t size)
{
    ftruncate(fd, size);
    return 0;
}

static int passfs_truncate(void * private,const char *path, off_t size)
{
    truncate(path, size);
    return 0;
}

static int passfs_symlink(void * private,const char *from, const char *to)
{
    symlink(from, to);
    return 0;
}
    // unsupported functions
static int passfs_link(void * private,const char *from, const char *to)
{
    link(from, to);
    return 0;
}

static int passfs_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    mknod(path, mode, dev);
    return 0;
}

static int passfs_chmod(void * private,const char *path, mode_t mode)
{
    chmod(path, mode);
    return 0;
}

static int passfs_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    return -1;
}

static int passfs_mount(void * private)
{
    return 0;
}

static int passfs_unmount(void * private)
{
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


int malloc_passfs(passfs_t **fs)
{
    passfs_t * entry = NULL;

    entry = malloc(sizeof(passfs_t));
    //entry->passfs_block;
    entry->vfs_op = &g_passfs_syscall_op;
    *fs = entry;
    return 0;
}

void free_passfs(passfs_t *fs)
{
    free(fs);
}

