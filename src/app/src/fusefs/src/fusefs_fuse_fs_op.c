#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "fusefs_log.h"
#include "fusefs_fs.h"
#include "fusefs_storage.h"


static void *fusefs_fuse_init(struct fuse_conn_info *conn,
                struct fuse_config *cfg)
{
    FUSEFS_TRACE("fusefs_fuse_init enter");
    conn->want |= FUSE_CAP_ATOMIC_O_TRUNC;
    FUSEFS_TRACE("fusefs_fuse_init exit");
    return 0;
}

static void fusefs_fuse_destroy(void *eh)
{
}

static int fusefs_fuse_statfs(const char *path, struct statvfs *s)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("fusefs_fuse_statfs enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->stat(storage, path, s);
    if (rc < 0) {
        FUSEFS_ERROR("stat errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_statfs ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_statfs exit %s", path);
    return rc;
}

static int fusefs_fuse_getattr(const char *path, struct stat * s, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("fusefs_fuse_getattr %s", path);
    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->getattr(storage, path, s);
    if (rc < 0) {
        FUSEFS_ERROR("getattr errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_getattr ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_getattr exit %s", path);
    return rc;
}

static int fusefs_fuse_access(const char *path, int mask)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("fusefs_fuse_access enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->access(storage, path, mask);
    if (rc < 0) {
        FUSEFS_ERROR("access errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_access ok %s", path);

l_out:
     FUSEFS_TRACE("fusefs_fuse_access exit %s", path);
    return rc;
}

static int fusefs_fuse_mkdir(const char *path, mode_t mode)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("fusefs_fuse_mkdir enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->mkdir(storage, path, mode);
     if (rc < 0) {
        FUSEFS_ERROR("mkdir errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_mkdir ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_mkdir exit %s", path);
    return rc;
}

static int fusefs_fuse_opendir(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage = NULL;
    fusefs_storage_dir_t * storage_dir = NULL;

    FUSEFS_TRACE("fusefs_fuse_opendir enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

     rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->opendir(storage, path, (void **)(&storage_dir));
    if (rc < 0) {
        FUSEFS_ERROR("opendir errno(%d)", rc);
        goto l_out;
    }

    fi->fh = (uint64_t)storage_dir;
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_opendir ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_opendir exit %s", path);
    return rc;
}

static int fusefs_fuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage = NULL;
    fusefs_storage_dir_t * dp = NULL;

    FUSEFS_TRACE("fusefs_fuse_releasedir enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    dp = (fusefs_storage_dir_t*)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->closedir(storage, dp);
   if (rc < 0) {
        FUSEFS_ERROR("closedir errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_releasedir ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_releasedir exit %s", path);
    return rc;
}

static int fusefs_fuse_readdir(
                const char * path, void * buff,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info * fi,
                enum fuse_readdir_flags flags)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    fusefs_storage_dir_t *storage_dir = NULL;
    struct dirent *de = NULL;
    struct stat st;

    FUSEFS_TRACE("fusefs_fuse_releasedir enter path %s", path);
    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    storage_dir = (fusefs_storage_dir_t*)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    while (storage->s_op->readdir(storage, (void*)storage_dir, (struct dirent **)(&de)) == 0) {
        if (strcmp(de->d_name, ".") == 0) {
            continue;
        }

        if (strcmp(de->d_name, "..") == 0) {
            continue;
        }
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        FUSEFS_TRACE("readdir path %s-%s", path, de->d_name);
        if (filler(buff, de->d_name, &st, 0, 0)) {
            break;
        }
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_releasedir ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_releasedir exit %s", path);
    return rc;
}

static int fusefs_fuse_rename(const char * from, const char * to, unsigned int flags)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_rename enter %s to %s", from, to);
 
    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, from);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, from);
        goto l_out;
    }
    rc = storage->s_op->infs(storage, to);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, to);
        goto l_out;
    }

    rc = storage->s_op->rename(storage, from, to);
    if (rc < 0) {
        FUSEFS_ERROR("rename errno(%d)", rc);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_rename ok %s to %s", from, to);

l_out:
    FUSEFS_TRACE("fusefs_fuse_rename exit %s to %s", from, to);
    return rc;
}

static int fusefs_fuse_unlink(const char *path)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_unlink enter %s", path);
    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->unlink(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("unlink errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_unlink exit %s", path);

l_out:
    return rc;
}

static int fusefs_fuse_open(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_open enter %s", path);
    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->open(storage, path, fi->flags, (void**)(&file));
    if (rc < 0) {
        FUSEFS_ERROR("open errno(%d) %s", rc, path);
        goto l_out;
    }

    fi->fh = (uint64_t)file;
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_open ok %s", path);
 
l_out:
     FUSEFS_TRACE("fusefs_fuse_open exit %s", path);
    return rc;
}

static int fusefs_fuse_release(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_release enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    file = (fusefs_storage_file_t *)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->close(storage, file);
    if (rc < 0) {
        FUSEFS_ERROR("close errno(%d), %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_release ok %s", path);

l_out:
     FUSEFS_TRACE("fusefs_fuse_release exit %s", path);
    return rc;
}

static int fusefs_fuse_read(const char *path, char *buf, size_t size,
        off_t off, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    ssize_t read_size = 0;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_read enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    file = (fusefs_storage_file_t *)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    read_size = storage->s_op->read(storage, file, buf, size, off);
    if (read_size < 0) {
        rc= (int)read_size;
        FUSEFS_ERROR("read errno(%d), %s", rc, path);
        goto l_out;
    }
    rc = (int)read_size;
    FUSEFS_TRACE("fusefs_fuse_read ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_read exit %s", path);
    return rc;
}

static int fusefs_fuse_write(const char *path, const char *buff, size_t size,
        off_t off, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    ssize_t write_isze = 0;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_read enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    file = (fusefs_storage_file_t *)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    write_isze = storage->s_op->write(storage, file, buff, size, off);
    if (write_isze < 0) {
        rc= (int)write_isze;
        FUSEFS_ERROR("write errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = (int)write_isze;
    FUSEFS_TRACE("fusefs_fuse_read exit %s", path);

l_out:
    return rc;

}

static int fusefs_fuse_fsync(const char *path, int isdatasync,
        struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_fsync enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    file = (fusefs_storage_file_t *)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->fsync(storage, file, 1);
    if (rc < 0) {
        FUSEFS_ERROR("fusefs_fuse_fsync errno(%d), %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_fsync ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_fsync exit %s", path);
    return rc;
}

static int fusefs_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_create enter %s", path);

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->open(storage, path, fi->flags, (void**)(&file));
    if (rc < 0) {
        FUSEFS_ERROR("open errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = storage->s_op->close(storage, file);
    if (rc < 0) {
        FUSEFS_ERROR("close errno(%d), %s", rc, path);
        goto l_out;
    }
    rc = storage->s_op->chmod(storage, path, mode);
    if (rc < 0) {
        FUSEFS_ERROR("chmod errno(%d), %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_create ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_create exit %s", path);
    return rc;
}

static int fusefs_fuse_ftruncate(const char *path, off_t size,
        struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;
    fusefs_storage_file_t * file = NULL;

    FUSEFS_TRACE("fusefs_fuse_create enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;
    file = (fusefs_storage_file_t *)fi->fh;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->ftruncate(storage, file, size);
     if (rc < 0) {
        FUSEFS_ERROR("ftruncate failed errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_create ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_create exit %s", path);
    return rc;
}

static int fusefs_fs_truncate(const char * path, off_t size, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_create enter");

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->truncate(storage, path, size);
     if (rc < 0) {
        FUSEFS_ERROR("truncate failed errno(%d)", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_create ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_create exit %s", path);
    return rc;
}

static int fusefs_fuse_symlink(const char *from, const char *to)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_symlink enter from %s to %s", from, to);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, from);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, from);
        goto l_out;
    }

    rc = storage->s_op->infs(storage, to);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, to);
        goto l_out;
    }

    rc = storage->s_op->symlink(storage, from, to);
    if (rc < 0) {
        FUSEFS_ERROR("fusefs_fs_truncate failed errno(%d)", rc);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("symlink exit %s to %s", from, to);
    
l_out:
    return rc;
}

static int fusefs_fuse_link(const char *from, const char *to)
{
    int rc =0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_link enter  %s to %s", from, to);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, from);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, from);
        goto l_out;
    }
    rc = storage->s_op->infs(storage, to);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, to);
        goto l_out;
    }

    rc = storage->s_op->link(storage, from, to);
    if (rc < 0) {
        FUSEFS_ERROR("link failed errno(%d)", rc);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_link ok %s to %s", from, to);
    
l_out:
     FUSEFS_TRACE("fusefs_fuse_link exit %s to %s", from, to);
    return rc;
}

static int fusefs_fuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    int rc =0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_mknod enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->mknod(storage, path, mode, dev);
    if (rc < 0) {
        FUSEFS_ERROR("mknod failed errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_mknod ok %s", path);
l_out:
    FUSEFS_TRACE("fusefs_fuse_mknod exit %s", path);
    return rc;
}

static int fusefs_fuse_chmod(const char * path, mode_t mode, struct fuse_file_info *fi)
{
    int rc =0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_chmod enter %s");
    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }
 
    rc = storage->s_op->chmod(storage, path, mode);
    if (rc < 0) {
        FUSEFS_ERROR("fusefs_fuse_chmod failed errno(%d)", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_chmod ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_chmod exit %s", path);
    return rc;
}

static int fusefs_fuse_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fs_t * fs = NULL;
    fusefs_storage_t * storage =NULL;

    FUSEFS_TRACE("fusefs_fuse_chown enter %s", path);

    fs = fusefs_get_fs();
    storage = fs->fusefs_storage;

    rc = storage->s_op->infs(storage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = storage->s_op->chown(storage, path, uid, gid);
    if (rc < 0) {
        FUSEFS_ERROR("fusefs_fuse_chmod failed errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fusefs_fuse_chown ok %s", path);

l_out:
    FUSEFS_TRACE("fusefs_fuse_chown exit %s", path);
    return rc;
}

static int fusefs_fuse_utimens(const char *path, const struct timespec ts[2], struct fuse_file_info *fi)
{
    FUSEFS_TRACE("fusefs_fuse_utimens enter %s", path);
    FUSEFS_TRACE("fusefs_fuse_utimens ok %s", path);
    FUSEFS_TRACE("fusefs_fuse_utimens exit %s", path);
    return 0;
}

static int fusefs_fuse_readlink(const char *path , char * buff, size_t size)
{
    return -1;
}

static int fusefs_fuse_flush(const char *path, struct fuse_file_info *fi)
{
    return -1;
}

static int fusefs_fuse_setxattr(const char * path, const char *key, const char * value,
                size_t size, int flags)
{
    return -1;
}

static int fusefs_fuse_getxattr(const char *path, const char * key, char * value, size_t size)
{
    return -1;
}

static int fusefs_fuse_listxattr(const char *path, char * key, size_t size)
{
    return -1;
}

static int fusefs_fuse_removexattr(const char *path, const char *key)
{
    return -1;
}

static int fusefs_fuse_fsyncdir(const char *path, int flag, struct fuse_file_info * fi)
{
    return -1;
}

static int fusefs_fuse_lock(const char * path, struct fuse_file_info * fi, int cmd,
                struct flock * lock)
{
    return -1;
}


static struct fuse_operations g_fusefs_fuse_op = {
    .init       = fusefs_fuse_init,
    .destroy    = fusefs_fuse_destroy,
    .statfs     = fusefs_fuse_statfs,

    .getattr    = fusefs_fuse_getattr,
    .access     = fusefs_fuse_access,

    .mkdir      = fusefs_fuse_mkdir,
    .rmdir      = fusefs_fuse_unlink,
    .opendir    = fusefs_fuse_opendir,
    .releasedir = fusefs_fuse_releasedir,
    .readdir    = fusefs_fuse_readdir,

    .rename     = fusefs_fuse_rename,
    .unlink     = fusefs_fuse_unlink,

    .open       = fusefs_fuse_open,
    .create     = fusefs_fuse_create,
    .truncate   = fusefs_fs_truncate,
    .release    = fusefs_fuse_release,
    .read       = fusefs_fuse_read,
    .write      = fusefs_fuse_write,
    .fsync      = fusefs_fuse_fsync,

    .link       = fusefs_fuse_link,
    .symlink    = fusefs_fuse_symlink,
    .readlink   = fusefs_fuse_readlink,
    .flush      = fusefs_fuse_flush,
    .mknod      = fusefs_fuse_mknod,
    .chmod      = fusefs_fuse_chmod,
    .chown      = fusefs_fuse_chown,
    .utimens    = fusefs_fuse_utimens,
};


struct fuse_operations * fusefs_get_fs_op()
{
    return &g_fusefs_fuse_op;
}


