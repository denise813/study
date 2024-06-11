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
#include "spfs_inter.h"


/* --comment by louting, 2024/5/13--
 * 
 */
#define EXTFS_PATH_MAX 1024

static int spfs_trans_path(void * private, const char * path, char * real_path)
{
    spfs_t * fs = (spfs_t*)private;
    spfs_config_t * config = &fs->spfs_config;

    snprintf(real_path, PASS_PATH_MAX, "%s/%s", config->spfs_bdev, path);
    return 0;
}

static int spfs_infs(void * private, const char * path)
{
    YSFS_TRACE("spfs_infs enter %s", path);
    YSFS_TRACE("spfs_infs exit %s", path);
    return 0;
}

static int spfs_nothing()
{
    return 0;
}

static int spfs_syscall_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_stat enter %s-%s", path, real_path);

    rc = storage_stat(real_path, s);
    if (rc < 0) {
        YSFS_ERROR("statvfs failed, errno(%d) %s-%s", rc, path, real_path);
        rc = 0;
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_stat ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_stat exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_getattr enter %s-%s", path, real_path);
    if (strcmp(real_path, config->spfs_bdev) == 0) {
        st->st_mode = 040755; // directory
        st->st_size = 0;
        st->st_uid = getuid();
        st->st_nlink = 1;
        goto l_out;
    }
    rc = storage_stat(real_path, s);
    if (rc < 0) {
        YSFS_ERROR("lstat failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_getattr ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_getattr exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_access(void * private, const char *path, int mask)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;
    spfs_inode_t* inode = NULL;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_access enter %s-%s", path, real_path);
    spfs_get_inode(&inode);
    int rc = spfs_tree_lookup(path, inode);
    if (rc != 0) {
        rc = -ENOENT;
        goto l_out;
    }

    inode->atime = time(NULL);
    spfs_put_inode(inode);
    rc = 0;
    YSFS_TRACE("spfs_access ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_access exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_mkdir enter %s-%s", path, real_path);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("mkdir failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_mkdir ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_mkdir exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    DIR * spfs_dp = NULL;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_opendir enter %s-%s", path, real_path);

    spfs_dp = spfs_nothing();
    if (!spfs_dp) {
        YSFS_ERROR("opendir failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    *dp = (void*)spfs_dp;
    YSFS_TRACE("spfs_opendir exit %s-%s %p", path, real_path, *dp);

l_out:
    YSFS_TRACE("spfs_opendir exit %s-%s %p", path, real_path);
    return rc;
}

static int spfs_syscall_closedir(void * private, void *dp)
{
    int rc = 0;
    DIR * spfs_dp = dp;

    YSFS_TRACE("spfs_closedir enter %p", dp);
    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("closedir failed %d %p", rc, dp);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_closedir ok %p", dp);

l_out:
    YSFS_TRACE("spfs_closedir exit %p", dp);
    return 0;
}

static int spfs_syscall_readdir(void * private, void *dp,
                    struct dirent **de)
{
    int rc = 0;
     DIR *spfs_dp = NULL;
    struct dirent * entry = NULL;

    YSFS_TRACE("spfs_readdir enter %p", dp);
    spfs_dp = (DIR*)dp;
    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("readdir failed %d", rc);
        goto l_out;
    }

    *de = entry;
    rc = 0;
    YSFS_TRACE("spfs_readdir ok %p", dp);

l_out:
    YSFS_TRACE("spfs_readdir exit %p", dp);
    return rc;
}

static int spfs_syscall_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    char spfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = spfs_path_from;
    char spfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = spfs_path_to;

    spfs_trans_path(private, to, spfs_path_from);
    spfs_trans_path(private, to, spfs_path_to);
    YSFS_TRACE("spfs_rename enter %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("rename failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        spfs_path_from,
                        spfs_path_to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_rename ok %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

l_out:
    YSFS_TRACE("spfs_rename exit %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);
    return rc;
}

static int spfs_syscall_unlink(void * private, const char *path)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_unlink enter. %s-%s", path, real_path);
 
    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("unlink failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_unlink ok. %s-%s", path, real_path);

l_out:
     YSFS_TRACE("spfs_unlink exit. %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;
    int *spfs_file = NULL;;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_open enter %s-%s", path, real_path);

    spfs_file = (int *)malloc(sizeof(int));
    if (!spfs_file) {
        YSFS_ERROR("malloc failed %d %s-%s", rc, path, real_path);
         goto l_out;
    }

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("open failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }
    rc = 0;
    *file = (void*)spfs_file;
    YSFS_TRACE("spfs_open ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_open exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_close(void * private, void *file)
{
    int rc = 0;
    int *spfs_file = NULL;

    YSFS_TRACE("spfs_close enter");

    spfs_file = (int*)(file);
    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("close failed %d", rc);
        goto l_out;
    }

     rc = 0;
    YSFS_TRACE("spfs_close exit");

l_out:
    YSFS_TRACE("spfs_close exit");
    return rc;
}

static ssize_t spfs_syscall_read(void * private, void * file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    int *spfs_file = NULL;
    ssize_t size = 0;

    YSFS_TRACE("spfs_read enter");

    spfs_file = (int*)(file);
    size = spfs_nothing();
    if (size < 0) {
        size  = rc;
        YSFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_read ok");

l_out:
    YSFS_TRACE("spfs_read exit");
    return size;
}

static ssize_t spfs_syscall_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    int *spfs_file = NULL;
    ssize_t size = 0;

    YSFS_TRACE("spfs_write enter");

    spfs_file = (int*)(file);
    size = spfs_nothing();
    if (size < 0) {
        size  = rc;
        YSFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_write ok");

l_out:
    YSFS_TRACE("spfs_write exit");
    return size;
}

static int spfs_syscall_fsync(void * private,
                void * file,
                int isdatasync)
{
    //fsync()
    YSFS_TRACE("spfs_fsync enter");
    YSFS_TRACE("spfs_fsync exit");
    return 0;
}

static int spfs_syscall_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    int *spfs_file = NULL;

    YSFS_TRACE("spfs_ftruncate enter");

    spfs_file = (int*)(file);
    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_ftruncate ok");

l_out:
    YSFS_TRACE("spfs_ftruncate exit");
    return rc;
}

static int spfs_syscall_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
   char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_truncate enter %s-%s", path, real_path);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("truncate failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_truncate ok %s-%s", path, real_path);

l_out:
     YSFS_TRACE("spfs_truncate exit %s-%s", path, real_path);
    return 0;
}

static int spfs_syscall_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    char spfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = spfs_path_from;
     char spfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = spfs_path_to;

    spfs_trans_path(private, from, spfs_path_from);
    spfs_trans_path(private, to, spfs_path_to);
    YSFS_TRACE("spfs_symlink enter %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("symlink failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        spfs_path_from,
                        spfs_path_to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_symlink ok %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

l_out:
    YSFS_TRACE("spfs_symlink exit %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);
    return rc;
}
    // unsupported functions
static int spfs_syscall_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    char spfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = spfs_path_from;
    char spfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = spfs_path_to;

    spfs_trans_path(private, from, real_path_from);
    spfs_trans_path(private, to, real_path_to);
    YSFS_TRACE("spfs_syscall_link enter %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_TRACE("link failed errno(%d) %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        spfs_path_from,
                        spfs_path_to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_syscall_link ok %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

l_out:
    YSFS_TRACE("spfs_syscall_link exit %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);
    return rc;
}

static int spfs_syscall_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_mknod enter %s-%s", path, real_path);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("spfs_mknod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_mknod ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_mknod exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_truncate enter %s-%s", path, real_path);

    rc = spfs_nothing();
    if (rc < 0) {
        YSFS_ERROR("spfs_truncate failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_truncate ok %s-%s", path, real_path);

l_out:
    YSFS_TRACE("spfs_truncate exit %s-%s", path, real_path);
    return rc;
}

static int spfs_syscall_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    YSFS_TRACE("spfs_chown enter %s-%s", path, real_path);

    rc = spfs_nothing();
     if (rc < 0) {
        YSFS_ERROR("spfs_chown failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("spfs_chown ok %s-%s", path, real_path);

l_out:
     YSFS_TRACE("spfs_chown exit %s-%s", path, real_path);
    return rc;
}

static int spfs_mount(void * private)
{
    spfs_t * fs = (spfs_t *)(private);
    YSFS_TRACE("spfs_mount enter ");

    spfs_blocks_init(fs->path);
    if (!spfs_bitmap_get(spfs_get_blocks_bitmap(), 1))
        for (int i = 0; i < 3; i++)
        {
            int new_block = spfs_alloc_block();
            printf("alloc inode block: %d\n", new_block);
        }

    if (!spfs_bitmap_get(spfs_get_blocks_bitmap(), 4))
    {
        printf("initializing root directory\n");
        directory_init();
    }
    YSFS_TRACE("spfs_mount exit");
    return 0;
}

static int spfs_unmount(void * private)
{
    spfs_t * fs = (spfs_t *)(private);
    YSFS_TRACE("spfs_unmount enter");

    YSFS_TRACE("spfs_unmount exit");
    return 0;
}

static vfs_syscall_t g_spfs_syscall_op =
{
    .stat = spfs_syscall_stat,
    .getattr = spfs_syscall_getattr,
    .access = spfs_syscall_access,
    .mkdir = spfs_syscall_mkdir,
    .opendir = spfs_syscall_opendir,
    .closedir = spfs_syscall_closedir,
    .readdir = spfs_syscall_readdir,
    .rename = spfs_syscall_rename,
    .unlink = spfs_syscall_unlink,
    .open = spfs_syscall_open,
    .close = spfs_syscall_close,
    .read = spfs_syscall_read,
    .write = spfs_syscall_write,
    .fsync = spfs_syscall_fsync,
    .ftruncate = spfs_syscall_ftruncate,
    .truncate = spfs_syscall_truncate,
    .link = spfs_syscall_link,
    .mknod = spfs_syscall_mknod,
    .chmod = spfs_syscall_chmod,
    .chown= spfs_syscall_chown,
    .mount = spfs_mount,
    .unmount = spfs_unmount,
};


int spfs_malloc_fs(spfs_t **fs)
{
    spfs_t * entry = NULL;

    YSFS_TRACE("malloc_spfs enter");
    entry = malloc(sizeof(spfs_t));
    //entry->spfs_block;
    entry->spfs_op = &g_spfs_syscall_op;
    
    *fs = entry;
    YSFS_TRACE("malloc_spfs exit");
    return 0;
}

void spfs_free_fs(spfs_t *fs)
{
    YSFS_TRACE("free_spfs enter");
    free(fs);
     YSFS_TRACE("free_spfs exit");
}

