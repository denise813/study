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
#include "libguestfs_inter.h"


/* --comment by louting, 2024/5/13--
 * 
 */
#define PASS_PATH_MAX 1024

static int libguestfs_trans_path(void * private, const char * path, char * real_path)
{
    libguestfs_t * fs = (libguestfs_t*)private;
    libguestfs_config_t * config = &fs->pfs_config;

    snprintf(real_path, PASS_PATH_MAX, "%s/%s", config->pfs_bdev, path);
    return 0;
}

static int libguestfs_infs(void * private, const char * path)
{
    FUSEFS_TRACE("libguestfs_infs enter %s", path);
    FUSEFS_TRACE("libguestfs_infs exit %s", path);
    return 0;
}

static int libguestfs_syscall_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_stat enter %s-%s", path, real_path);

    rc = statvfs(real_path, s);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("statvfs failed, errno(%d) %s-%s", rc, path, real_path);
        rc = 0;
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_stat ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_stat exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_getattr enter %s-%s", path, real_path);
    rc = lstat(real_path, s);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("lstat failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_getattr ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_getattr exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_access(void * private, const char *path, int mask)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_access enter %s-%s", path, real_path);

    rc = access(real_path, mask);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("libguestfs_access ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_access exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_mkdir enter %s-%s", path, real_path);

    rc = mkdir(real_path, mode);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_mkdir ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_mkdir exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    DIR * libguestfs_dp = NULL;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_opendir enter %s-%s", path, real_path);

    libguestfs_dp = opendir(real_path);
    if (!libguestfs_dp) {
        rc = -errno;
        FUSEFS_ERROR("opendir failed %d %s-%s", rc, path, real_path);
        rc = -1;
        goto l_out;
    }

    rc = 0;
    *dp = (void*)libguestfs_dp;
    FUSEFS_TRACE("libguestfs_opendir exit %s-%s %p", path, real_path, *dp);

l_out:
    FUSEFS_TRACE("libguestfs_opendir exit %s-%s %p", path, real_path);
    return rc;
}

static int libguestfs_syscall_closedir(void * private, void *dp)
{
    int rc = 0;
    DIR * libguestfs_dp = dp;

    FUSEFS_TRACE("libguestfs_closedir enter %p", dp);
    rc = closedir(libguestfs_dp);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("closedir failed %d %p", rc, dp);
        rc = -1;
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_closedir ok %p", dp);

l_out:
    FUSEFS_TRACE("libguestfs_closedir exit %p", dp);
    return 0;
}

static int libguestfs_syscall_readdir(void * private, void *dp,
                    struct dirent **de)
{
    int rc = 0;
     DIR *libguestfs_dp = NULL;
    struct dirent * entry = NULL;

    FUSEFS_TRACE("libguestfs_readdir enter %p", dp);
    libguestfs_dp = (DIR*)dp;
    entry = readdir(libguestfs_dp);
    if (!entry) {
        rc = -errno;
        FUSEFS_ERROR("readdir failed %d", rc);
        rc = -1;
        goto l_out;
    }

    *de = entry;
    rc = 0;
    FUSEFS_TRACE("libguestfs_readdir ok %p", dp);

l_out:
    FUSEFS_TRACE("libguestfs_readdir exit %p", dp);
    return rc;
}

static int libguestfs_syscall_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    char libguestfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = libguestfs_path_from;
    char libguestfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = libguestfs_path_to;

    libguestfs_trans_path(private, to, libguestfs_path_from);
    libguestfs_trans_path(private, to, libguestfs_path_to);
    FUSEFS_TRACE("libguestfs_rename enter %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

    rc = rename(libguestfs_path_from, libguestfs_path_to);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("rename failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        libguestfs_path_from,
                        libguestfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_rename ok %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

l_out:
    FUSEFS_TRACE("libguestfs_rename exit %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);
    return rc;
}

static int libguestfs_syscall_unlink(void * private, const char *path)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_unlink enter. %s-%s", path, real_path);
 
    rc = unlink(real_path);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("unlink failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_unlink ok. %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("libguestfs_unlink exit. %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    int *libguestfs_file = NULL;;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_open enter %s-%s", path, real_path);

    libguestfs_file = (int *)malloc(sizeof(int));
    if (!libguestfs_file) {
        rc = -ENOMEM;
        FUSEFS_ERROR("malloc failed %d %s-%s", rc, path, real_path);
         goto l_out;
    }

    *libguestfs_file = open(real_path, flags);
    if (*libguestfs_file < 0) {
        rc = -errno;
        FUSEFS_ERROR("open failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    *file = (void*)libguestfs_file;
    FUSEFS_TRACE("libguestfs_open ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_open exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_close(void * private, void *file)
{
    int rc = 0;
    int *libguestfs_file = NULL;

    FUSEFS_TRACE("libguestfs_close enter");

    libguestfs_file = (int*)(file);
    rc = close(*libguestfs_file);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("close failed %d", rc);
        goto l_out;
    }

     rc = 0;
    FUSEFS_TRACE("libguestfs_close exit");

l_out:
    FUSEFS_TRACE("libguestfs_close exit");
    return rc;
}

static ssize_t libguestfs_syscall_read(void * private, void * file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    int *libguestfs_file = NULL;
    ssize_t size = 0;

    FUSEFS_TRACE("libguestfs_read enter");

    libguestfs_file = (int*)(file);
    size = read(*libguestfs_file, buff, buff_size);
    if (size < 0) {
        rc = -errno;
        size  = rc;
        FUSEFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_read ok");

l_out:
    FUSEFS_TRACE("libguestfs_read exit");
    return size;
}

static ssize_t libguestfs_syscall_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    int *libguestfs_file = NULL;
    ssize_t size = 0;

    FUSEFS_TRACE("libguestfs_write enter");

    libguestfs_file = (int*)(file);
    size = write(*libguestfs_file, buff, buff_size);
    if (size < 0) {
        rc = -errno;
        size  = rc;
        FUSEFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_write ok");

l_out:
    FUSEFS_TRACE("libguestfs_write exit");
    return size;
}

static int libguestfs_syscall_fsync(void * private,
                void * file,
                int isdatasync)
{
    //fsync()
    FUSEFS_TRACE("libguestfs_fsync enter");
    FUSEFS_TRACE("libguestfs_fsync exit");
    return 0;
}

static int libguestfs_syscall_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    int *libguestfs_file = NULL;

    FUSEFS_TRACE("libguestfs_ftruncate enter");

    libguestfs_file = (int*)(file);
    rc = ftruncate(*libguestfs_file, size);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_ftruncate ok");

l_out:
    FUSEFS_TRACE("libguestfs_ftruncate exit");
    return rc;
}

static int libguestfs_syscall_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
   char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_truncate enter %s-%s", path, real_path);

    rc = truncate(real_path, size);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("truncate failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_truncate ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("libguestfs_truncate exit %s-%s", path, real_path);
    return 0;
}

static int libguestfs_syscall_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    char libguestfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = libguestfs_path_from;
     char libguestfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = libguestfs_path_to;

    libguestfs_trans_path(private, from, libguestfs_path_from);
    libguestfs_trans_path(private, to, libguestfs_path_to);
    FUSEFS_TRACE("libguestfs_symlink enter %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

    rc = symlink(libguestfs_path_from, libguestfs_path_to);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("symlink failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        libguestfs_path_from,
                        libguestfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_symlink ok %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

l_out:
    FUSEFS_TRACE("libguestfs_symlink exit %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);
    return rc;
}
    // unsupported functions
static int libguestfs_syscall_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    char libguestfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = libguestfs_path_from;
    char libguestfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = libguestfs_path_to;

    libguestfs_trans_path(private, from, real_path_from);
    libguestfs_trans_path(private, to, real_path_to);
    FUSEFS_TRACE("libguestfs_syscall_link enter %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

    rc = link(real_path_from, real_path_to);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_TRACE("link failed errno(%d) %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        libguestfs_path_from,
                        libguestfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_syscall_link ok %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

l_out:
    FUSEFS_TRACE("libguestfs_syscall_link exit %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);
    return rc;
}

static int libguestfs_syscall_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_mknod enter %s-%s", path, real_path);

    rc = mknod(real_path, mode, dev);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("libguestfs_mknod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_mknod ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_mknod exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_truncate enter %s-%s", path, real_path);

    rc = chmod(real_path, mode);
    if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("libguestfs_truncate failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_truncate ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_truncate exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_syscall_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    char libguestfs_path[PASS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_chown enter %s-%s", path, real_path);

    rc = chown(path, uid, gid);
     if (rc < 0) {
        rc = -errno;
        FUSEFS_ERROR("libguestfs_chown failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_chown ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("libguestfs_chown exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_mount(void * private)
{
    libguestfs_t * fs = (libguestfs_t *)(private);
    FUSEFS_TRACE("libguestfs_mount enter ");
    FUSEFS_TRACE("libguestfs_mount exit");
    return 0;
}

static int libguestfs_unmount(void * private)
{
    libguestfs_t * fs = (libguestfs_t *)(private);
    FUSEFS_TRACE("libguestfs_unmount enter");

    FUSEFS_TRACE("libguestfs_unmount exit");
    return 0;
}

static vfs_syscall_t g_libguestfs_syscall_op =
{
    .stat = libguestfs_syscall_stat,
    .getattr = libguestfs_syscall_getattr,
    .access = libguestfs_syscall_access,
    .mkdir = libguestfs_syscall_mkdir,
    .opendir = libguestfs_syscall_opendir,
    .closedir = libguestfs_syscall_closedir,
    .readdir = libguestfs_syscall_readdir,
    .rename = libguestfs_syscall_rename,
    .unlink = libguestfs_syscall_unlink,
    .open = libguestfs_syscall_open,
    .close = libguestfs_syscall_close,
    .read = libguestfs_syscall_read,
    .write = libguestfs_syscall_write,
    .fsync = libguestfs_syscall_fsync,
    .ftruncate = libguestfs_syscall_ftruncate,
    .truncate = libguestfs_syscall_truncate,
    .link = libguestfs_syscall_link,
    .mknod = libguestfs_syscall_mknod,
    .chmod = libguestfs_syscall_chmod,
    .chown= libguestfs_syscall_chown,
    .mount = libguestfs_mount,
    .unmount = libguestfs_unmount,
    .infs = libguestfs_infs,
};


int libguestfs_malloc_fs(void **fs, char * dev)
{
    libguestfs_t * entry = NULL;

    FUSEFS_TRACE("malloc_libguestfs enter");
    entry = malloc(sizeof(libguestfs_t));
    //entry->libguestfs_block;
    entry->pfs_op = &g_libguestfs_syscall_op;
    entry->pfs_config.pfs_bdev = strdup(dev);
    
    *fs = (void*)entry;
    FUSEFS_TRACE("malloc_libguestfs exit");
    return 0;
}

void libguestfs_free_fs(libguestfs_t *fs)
{
    FUSEFS_TRACE("free_libguestfs enter");
    free(fs->pfs_config.pfs_bdev);
    free(fs);
     FUSEFS_TRACE("free_libguestfs exit");
}

