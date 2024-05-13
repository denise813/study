#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "fusefs_fuse.h"


static void *fusefs_fuse_init(struct fuse_conn_info *conn,
                struct fuse_config *cfg)
{
    conn->want |= FUSE_CAP_ATOMIC_O_TRUNC;
    return 0;
}

static void fusefs_fuse_destroy(void *eh)
{
}

static int fusefs_fuse_statfs(const char *path, struct statvfs *s)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->stat(vfs->vfs_private, path, s);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_getattr(const char *path, struct stat * s, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->getattr(vfs->vfs_private, path, s);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_access(const char *path, int mask)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->access(vfs->vfs_private, path, mask);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_mkdir(const char *path, mode_t mode)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    rc = vfs->vfs_op->mkdir(vfs->vfs_private, path, mode);
     if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_opendir(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
     DIR *dp = NULL;

    rc = vfs->vfs_op->opendir(vfs->vfs_private, path, &dp);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    fi->fh = (uint64_t)dp;
    rc = 0;

l_out:
    return 0;
}

static int fusefs_fuse_releasedir(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    
    DIR *dp = (DIR*)fi->fh;
    rc = vfs->vfs_op->closedir(vfs->vfs_private, dp);
   if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
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
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    DIR *dp = (DIR*)fi->fh;
    struct dirent *de = NULL;
    struct stat st;

     while (!vfs->vfs_op->readdir(vfs->vfs_private, dp, &de)) {
        memset(&st, 0, sizeof(st));
        st.st_ino = de->d_ino;
        st.st_mode = de->d_type << 12;
        if (filler(buff, de->d_name, &st, 0, 0)) {
            break;
        }
    }
    return 0;
}

static int fusefs_fuse_rename(const char * from, const char * to, unsigned int flags)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    rc = vfs->vfs_op->rename(vfs->vfs_private, from, to);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_unlink(const char *path)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->unlink(vfs->vfs_private, path);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_open(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    fi->fh = -1;
    int * fd = (int *)malloc(sizeof(int));
    *fd = -1;
    rc = vfs->vfs_op->open(vfs->vfs_private, path, fi->flags, fd);
    if (*fd == -1) {
        rc =  -errno;
        goto l_free;
    }

    fi->fh = (uint64_t)fd;
    rc = 0;
l_out:
    return rc;
l_free:
    free(fd);
    fd = NULL;
    goto l_out;
}

static int fusefs_fuse_release(const char *path, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    int * fd = (int *)fi->fh;
     rc = vfs->vfs_op->close(vfs->vfs_private, *fd);
    if (rc == -1) {
        *fd = -1;
        rc = -errno;
        goto l_free;
    }
    *fd = -1;
    free(fd);
    rc = 0;

l_out:
    return rc;
l_free:
    *fd = -1;
    free(fd);
    goto l_out;
}

static int fusefs_fuse_read(const char *path, char *buf, size_t size,
        off_t off, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    ssize_t read_isze = 0;
    int * fd = (int *)fi->fh;

    read_isze = vfs->vfs_op->read(vfs->vfs_private, *fd, buf, size, off);
    if (read_isze < 0) {
        rc= (int)read_isze;
        goto l_out;
    }
    rc = (int)read_isze;

l_out:
     return rc;
}

static int fusefs_fuse_write(const char *path, const char *buff, size_t size,
        off_t off, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    ssize_t write_isze = 0;
    int * fd = (int *)fi->fh;

    write_isze = vfs->vfs_op->write(vfs->vfs_private, *fd, buff, size, off);
    if (write_isze < 0) {
        rc= (int)write_isze;
        goto l_out;
    }
    rc = (int)write_isze;

l_out:
    return rc;

}

static int fusefs_fuse_fsync(const char *path, int isdatasync,
        struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    int * fd = (int *)fi->fh;

    rc = vfs->vfs_op->fsync(vfs->vfs_private, *fd, 1);
    if (rc < 0) {
        goto l_out;
    }
    rc = 0;
l_out:
    return rc;
}

static int fusefs_fuse_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
    int rc = 0;
    rc = fusefs_fuse_open(path, fi);
    if (rc < 0) {
        goto l_out;
    }
    fusefs_fuse_release(path, fi);
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_ftruncate(const char *path, off_t size,
        struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;
    int * fd = (int *)fi->fh;

    rc = vfs->vfs_op->ftruncate(vfs->vfs_private, *fd, size);
     if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_truncate(const char * path, off_t size, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->truncate(vfs->vfs_private, path, size);
     if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int fusefs_fuse_symlink(const char *from, const char *to)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->symlink(vfs->vfs_private, from, to);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;
    
l_out:
    return rc;
}

static int fusefs_fuse_link(const char *from, const char *to)
{
    int rc =0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->link(vfs->vfs_private, from, to);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;
    
l_out:
    return rc;
}

static int fusefs_fuse_mknod(const char *path, mode_t mode, dev_t dev)
{
    // not supported, fail
    int rc =0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->mknod(vfs->vfs_private, path, mode, dev);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;
l_out:
    return rc;
}

static int fusefs_fuse_chmod(const char * path, mode_t mode, struct fuse_file_info *fi)
{
    int rc =0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->chmod(vfs->vfs_private, path, mode);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;
l_out:
    return rc;
}

static int fusefs_fuse_chown(const char *path, uid_t uid, gid_t gid, struct fuse_file_info *fi)
{
    int rc = 0;
    fusefs_fuse_fs_t * fs = get_fusefs_fuse_fs();
    vfs_t * vfs = fs->ffs_vfs;

    rc = vfs->vfs_op->chown(vfs->vfs_private, path, uid, gid);
    if (rc < 0) {
        rc = -errno;
        goto l_out;
    }
    rc = 0;
l_out:
    return rc;
}

static int fusefs_fuse_utimens(const char *path, const struct timespec ts[2], struct fuse_file_info *fi)
{
    return 0;
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
    .truncate   = fusefs_fuse_truncate,
    .release    = fusefs_fuse_release,
    .read       = fusefs_fuse_read,
    .write      = fusefs_fuse_write,
    .fsync      = fusefs_fuse_fsync,

    .link       = fusefs_fuse_link,
    .symlink    = fusefs_fuse_symlink,
    .mknod      = fusefs_fuse_mknod,
    .chmod      = fusefs_fuse_chmod,
    .chown      = fusefs_fuse_chown,
    .utimens    = fusefs_fuse_utimens,
};


struct fuse_operations * get_fusefs_fuse_fs_op()
{
    return &g_fusefs_fuse_op;
}


