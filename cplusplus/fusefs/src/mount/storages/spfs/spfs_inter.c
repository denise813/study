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
#define SPFS_PATH_MAX 1024

static int spfs_trans_path(void * private, const char * path, char * real_path)
{
    spfs_t * fs = (spfs_t*)private;
    spfs_config_t * config = &fs->spfs_config;

    snprintf(real_path, PASS_PATH_MAX, "%s/%s", config->spfs_bdev, path);
    return 0;
}

static int spfs_infs(void * private, const char * path)
{
    FUSEFS_TRACE("spfs_infs enter %s", path);
    FUSEFS_TRACE("spfs_infs exit %s", path);
    return 0;
}

static int spfs_nothing()
{
    return 0;
}


static int spfs_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_stat enter %s-%s", path, real_path);

    rc = storage_stat(real_path, s);
    if (rc < 0) {
        FUSEFS_ERROR("statvfs failed, errno(%d) %s-%s", rc, path, real_path);
        rc = 0;
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_stat ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_stat exit %s-%s", path, real_path);
    return rc;
}

static int spfs_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_getattr enter %s-%s", path, real_path);
    if (strcmp(real_path, config->spfs_bdev) == 0) {
        st->st_mode = 040755; // directory
        st->st_size = 0;
        st->st_uid = getuid();
        st->st_nlink = 1;
        goto l_out;
    }
    rc = storage_stat(real_path, s);
    if (rc < 0) {
        FUSEFS_ERROR("lstat failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_getattr ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_getattr exit %s-%s", path, real_path);
    return rc;
}

static int spfs_access(void * private, const char *path, int mask)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;
    spfs_inode_t* inode = NULL;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_access enter %s-%s", path, real_path);
    spfs_get_inode(&inode);
    int rc = spfs_tree_lookup(path, inode);
    if (rc != 0) {
        rc = -ENOENT;
        goto l_out;
    }

    inode->atime = time(NULL);
    spfs_put_inode(inode);
    rc = 0;
    FUSEFS_TRACE("spfs_access ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_access exit %s-%s", path, real_path);
    return rc;
}

static int spfs_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_mkdir enter %s-%s", path, real_path);

    rc = nufs_mknod(path, mode | 040000, 0);
    if (rc < 0) {
        FUSEFS_ERROR("mkdir failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_mkdir ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_mkdir exit %s-%s", path, real_path);
    return rc;
}

static int spfs_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    spfs_dir_t * spfs_dp = NULL;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_opendir enter %s-%s", path, real_path);

    spfs_dp = (spfs_dir_t*)malloc(sizeof(spfs_dir_t));
    if (!spfs_dp) {
        rc = -ENOMEM;
        FUSEFS_ERROR("opendir failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }
    spfs_dp->d_path = strdup(path);
    spfs_dp->d_itor = (struct dirent *)malloc(sizeof(struct dirent));
     if (!spfs_dp) {
        rc = -ENOMEM;
        FUSEFS_ERROR("opendir failed %d %s-%s", rc, path, real_path);
        goto l_free_dir;
    }

    rc = 0;
    *dp = (void*)spfs_dp;
    FUSEFS_TRACE("spfs_opendir exit %s-%s %p", path, real_path, *dp);

l_out:
    FUSEFS_TRACE("spfs_opendir exit %s-%s %p", path, real_path);
    return rc;

l_free_dir:
    free(spfs_dp);
    goto l_out;
}

static int spfs_closedir(void * private, void *dp)
{
    int rc = 0;
    spfs_dir_t * spfs_dp = dp;

    FUSEFS_TRACE("spfs_closedir enter %p", dp);
    free(spfs_dp->d_path);
    free(spfs_dp->d_itor);
    free(spfs_dp);
    rc = 0;
    FUSEFS_TRACE("spfs_closedir ok %p", dp);

l_out:
    FUSEFS_TRACE("spfs_closedir exit %p", dp);
    return 0;
}

static int spfs_readdir(void * private, void *dp,
                    struct dirent **de)
{
    int rc = 0;
    spfs_dir_t *spfs_dp = NULL;
    struct dirent * entry = NULL;
    int path_len = 0;
    struct stat st;
    char* temp_path = NULL;

    FUSEFS_TRACE("spfs_readdir enter %p", dp);
    rc = spfs_getattr(spfs_dp->d_path, &st);
    if (rc < 0) {
        goto l_out;
    }

    if (!spfs_dp->d_slist) {
        spfs_dp->d_slist = storage_list(spfs_dp->d_path);
        spfs_dp->d_index = spfs_dp->d_slist;
    }
    if (!spfs_dp->d_slist) {
        rc = -1;
        goto l_out;
    }
    if (spfs_dp->d_index->next) {
        rc = 0;
        goto l_out;
    }

    spfs_dp->d_itor.d_ino = spfs_dp->d_index.data->d_ino;
    memcpy(spfs_dp->d_itor->d_name, spfs_dp->d_index->data->name, 512);
    switch (spfs_dp->d_index->data.ftype) {
        case 'b': spfs_dp->d_itor->d_type = DT_BLK; break;
        case 'c': spfs_dp->d_itor->d_type = DT_CHR; break;
        case 'd': spfs_dp->d_itor->d_type = DT_DIR; break;
        case 'f': spfs_dp->d_itor->d_type = DT_FIFO; break;
        case 'l': spfs_dp->d_itor->d_type = DT_LNK; break;
        case 'r': spfs_dp->d_itor->d_type = DT_REG; break;
        case 's': spfs_dp->d_itor->d_type = DT_SOCK; break;
        case 'u':
        case '?':
        default:  spfs_dp->d_itor->d_type = DT_UNKNOWN;
    }

    *de = entry;
    rc = 0;
    FUSEFS_TRACE("spfs_readdir ok %p", dp);

l_out:
    FUSEFS_TRACE("spfs_readdir exit %p", dp);
    return rc;
}

static int spfs_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    char spfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = spfs_path_from;
    char spfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = spfs_path_to;

    spfs_trans_path(private, to, spfs_path_from);
    spfs_trans_path(private, to, spfs_path_to);
    FUSEFS_TRACE("spfs_rename enter %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

    rc = storage_rename(spfs_path_from, spfs_path_to);
    if (rc < 0) {
        FUSEFS_ERROR("rename failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        spfs_path_from,
                        spfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_rename ok %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

l_out:
    FUSEFS_TRACE("spfs_rename exit %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);
    return rc;
}

static int spfs_unlink(void * private, const char *path)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_unlink enter. %s-%s", path, real_path);
 
    rc = storage_unlink(path);
    if (rc < 0) {
        FUSEFS_ERROR("unlink failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_unlink ok. %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("spfs_unlink exit. %s-%s", path, real_path);
    return rc;
}

static int spfs_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;
    spfs_file_t *spfs_file = NULL;;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_open enter %s-%s", path, real_path);

    spfs_file = (spfs_file_t *)malloc(sizeof(spfs_file_t));
    if (!spfs_file) {
        FUSEFS_ERROR("malloc failed %d %s-%s", rc, path, real_path);
         goto l_out;
    }

    spfs_file->f_path = strdup(path);

    rc = 0;
    *file = (void*)spfs_file;
    FUSEFS_TRACE("spfs_open ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_open exit %s-%s", path, real_path);
    return rc;
}

static int spfs_close(void * private, void *file)
{
    spfs_file_t *spfs_file = NULL;

    FUSEFS_TRACE("spfs_close enter");

    spfs_file = (spfs_file_t*)(file);
    free(spfs_file->f_path);
    free(spfs_file);
    FUSEFS_TRACE("spfs_close ok");

l_out:
    FUSEFS_TRACE("spfs_close exit");
    return 0;
}

static ssize_t spfs_read(void * private, void * file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    spfs_file_t *spfs_file = NULL;
    ssize_t size = 0;

    FUSEFS_TRACE("spfs_read enter");

    spfs_file = (spfs_file_t*)(file);
    size = storage_read(spfs_file->f_path, buf, size, offset);
    if (size < 0) {
        size  = rc;
        FUSEFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_read ok");

l_out:
    FUSEFS_TRACE("spfs_read exit");
    return size;
}

static ssize_t spfs_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    spfs_file_t *spfs_file = NULL;
    ssize_t size = 0;

    FUSEFS_TRACE("spfs_write enter");

    spfs_file = (spfs_file_t*)(file);
    size = storage_write(spfs_file->f_path, buff_size, size, off);
    if (size < 0) {
        size  = rc;
        FUSEFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_write ok");

l_out:
    FUSEFS_TRACE("spfs_write exit");
    return size;
}

static int spfs_fsync(void * private,
                void * file,
                int isdatasync)
{
    //fsync()
    FUSEFS_TRACE("spfs_fsync enter");
    FUSEFS_TRACE("spfs_fsync exit");
    return 0;
}

static int spfs_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    int *spfs_file = NULL;

    FUSEFS_TRACE("spfs_ftruncate enter");

    spfs_file = (int*)(file);
    rc = spfs_nothing();
    if (rc < 0) {
        FUSEFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_ftruncate ok");

l_out:
    FUSEFS_TRACE("spfs_ftruncate exit");
    return rc;
}

static int spfs_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
   char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_truncate enter %s-%s", path, real_path);

    rc = storage_truncate(path, size);
    if (rc < 0) {
        FUSEFS_ERROR("truncate failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_truncate ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("spfs_truncate exit %s-%s", path, real_path);
    return 0;
}

static int spfs_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    char spfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = spfs_path_from;
     char spfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = spfs_path_to;

    spfs_trans_path(private, from, spfs_path_from);
    spfs_trans_path(private, to, spfs_path_to);
    FUSEFS_TRACE("spfs_symlink enter %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

    rc = spfs_nothing();
    if (rc < 0) {
        FUSEFS_ERROR("symlink failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        spfs_path_from,
                        spfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_symlink ok %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

l_out:
    FUSEFS_TRACE("spfs_symlink exit %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);
    return rc;
}
    // unsupported functions
static int spfs_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    char spfs_path_from[PASS_PATH_MAX] = {0};
    char * real_path_from = spfs_path_from;
    char spfs_path_to[PASS_PATH_MAX] = {0};
    char * real_path_to = spfs_path_to;

    spfs_trans_path(private, from, real_path_from);
    spfs_trans_path(private, to, real_path_to);
    FUSEFS_TRACE("spfs_link enter %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

    rc = storage_link(from, to);
    if (rc < 0) {
        FUSEFS_TRACE("link failed errno(%d) %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        spfs_path_from,
                        spfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_link ok %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);

l_out:
    FUSEFS_TRACE("spfs_link exit %s-%s, %s-%s",
                    from,
                    to,
                    spfs_path_from,
                    spfs_path_to);
    return rc;
}

static int spfs_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_mknod enter %s-%s", path, real_path);

    rc = storage_mknod(real_path, mode);
    if (rc < 0) {
        FUSEFS_ERROR("spfs_mknod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_mknod ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_mknod exit %s-%s", path, real_path);
    return rc;
}

static int spfs_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_truncate enter %s-%s", path, real_path);

    rc = storage_chmod(path, mode);
    if (rc < 0) {
        FUSEFS_ERROR("spfs_truncate failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_truncate ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("spfs_truncate exit %s-%s", path, real_path);
    return rc;
}

static int spfs_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    char spfs_path[PASS_PATH_MAX] = {0};
    char * real_path = spfs_path;

    spfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("spfs_chown enter %s-%s", path, real_path);

    rc = spfs_nothing();
     if (rc < 0) {
        FUSEFS_ERROR("spfs_chown failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("spfs_chown ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("spfs_chown exit %s-%s", path, real_path);
    return rc;
}

static int spfs_mount(void * private)
{
    spfs_t * fs = (spfs_t *)(private);
    FUSEFS_TRACE("spfs_mount enter ");

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
    FUSEFS_TRACE("spfs_mount exit");
    return 0;
}

static int spfs_unmount(void * private)
{
    spfs_t * fs = (spfs_t *)(private);
    FUSEFS_TRACE("spfs_unmount enter");

    FUSEFS_TRACE("spfs_unmount exit");
    return 0;
}

static storage_op_t g_spfs_op =
{
    .stat = spfs_stat,
    .getattr = spfs_getattr,
    .access = spfs_access,
    .mkdir = spfs_mkdir,
    .opendir = spfs_opendir,
    .closedir = spfs_closedir,
    .readdir = spfs_readdir,
    .rename = spfs_rename,
    .unlink = spfs_unlink,
    .open = spfs_open,
    .close = spfs_close,
    .read = spfs_read,
    .write = spfs_write,
    .fsync = spfs_fsync,
    .ftruncate = spfs_ftruncate,
    .truncate = spfs_truncate,
    .link = spfs_link,
    .mknod = spfs_mknod,
    .chmod = spfs_chmod,
    .chown= spfs_chown,
    .mount = spfs_mount,
    .unmount = spfs_unmount,
};


int spfs_malloc_fs(spfs_t **fs)
{
    spfs_t * entry = NULL;

    FUSEFS_TRACE("malloc_spfs enter");
    entry = malloc(sizeof(spfs_t));
    //entry->spfs_block;
    entry->spfs_op = &g_spfs_op;
    
    *fs = entry;
    FUSEFS_TRACE("malloc_spfs exit");
    return 0;
}

void spfs_free_fs(spfs_t *fs)
{
    FUSEFS_TRACE("free_spfs enter");
    free(fs);
    FUSEFS_TRACE("free_spfs exit");
}

