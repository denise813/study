#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "vfs_op.h"
#include "vfs.h"
#include "src/ysfs_log.h"
#include "src/fs/passfs/passfs.h"


static int vfs_infs(void * private,const char * path)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_infs enter %s", path);

    vfs = (vfs_t*)(private);
     rc = vfs->vfs_agentfs.afs_op->infs(vfs->vfs_agentfs.afs_private, path);
    if (rc < 0) {
        YSFS_ERROR("infs failed, errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_infs ok %s", path);

l_out:
    YSFS_TRACE("vfs_infs exit %s", path);
    return 0;
}

static int vfs_syscall_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_stat enter %s", path);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->stat(vfs->vfs_agentfs.afs_private, path, s);
    if (rc < 0) {
        YSFS_ERROR("stat failed, errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("vfs_syscall_stat ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_stat exit %s", path);
    return rc;
}

static int vfs_syscall_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_getattr enter %s", path);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->getattr(vfs->vfs_agentfs.afs_private, path, s);
    if (rc < 0) {
        YSFS_ERROR("getattr failed, errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_getattr ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_getattr exit %s", path);
    return rc;
}

static int vfs_syscall_access(void * private, const char *path, int mask)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("passfs_access enter %s", path);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->access(vfs->vfs_agentfs.afs_private, path, mask);
    if (rc < 0) {
        YSFS_ERROR("access failed, errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_access ok %s", path);

l_out:
    YSFS_TRACE("passfs_access exit %s", path);
    return 0;
}

static int vfs_syscall_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    vfs_t * vfs = NULL;
 
    YSFS_TRACE("vfs_syscall_mkdir enter %s", path);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->mkdir(vfs->vfs_agentfs.afs_private, path, mode);
     if (rc < 0) {
        YSFS_ERROR("mkdir failed, errno(%d) %s", rc, path);
        goto l_out;
    }
     
    rc = 0;
    YSFS_TRACE("vfs_syscall_mkdir ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_mkdir exit %s", path);
    return rc;
}

static int vfs_syscall_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_dir_t * vfs_dir = NULL;

    YSFS_TRACE("vfs_syscall_opendir enter %s", path);

    vfs_dir = (vfs_dir_t*)malloc(sizeof(vfs_dir_t));
    if (!vfs_dir) {
        rc = -ENOMEM;
        YSFS_ERROR("malloc failed, errno(%d) %s", rc, path);
        goto l_out;
    }
    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->opendir(vfs->vfs_agentfs.afs_private,
                    path,
                    &vfs_dir->d_private);
    if (rc < 0) {
        YSFS_ERROR("opendir failed %d %s", rc, path);
        rc = -1;
        goto l_free;
    }

    *dp = (void*)vfs_dir;
    rc = 0;
    YSFS_TRACE("vfs_syscall_opendir ok %s %p", path, *dp);

l_out:
    YSFS_TRACE("vfs_syscall_opendir exit %s %p", path, *dp);
    return rc;
l_free:
    free(vfs_dir);
    vfs_dir = NULL;
    goto l_out;
}

static int vfs_syscall_closedir(void * private, void *dp)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_dir_t * vfs_dir = NULL;

    YSFS_TRACE("passfs_closedir enter %p", dp);

    vfs = (vfs_t*)(private);
    vfs_dir = (vfs_dir_t*)(dp);
    rc = vfs->vfs_agentfs.afs_op->closedir(vfs->vfs_agentfs.afs_private,
                    vfs_dir->d_private);
     if (rc < 0) {
        YSFS_ERROR("closedir failed %d %p", rc, dp);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_closedir ok %p", dp);

l_out:
    YSFS_TRACE("passfs_closedir exit %p", dp);
    return rc;
}

static int vfs_syscall_readdir(void * private, void *dp, struct dirent ** d_itor)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_dir_t * vfs_dir = NULL;
    struct dirent * vfs_d_itor = NULL;

    YSFS_TRACE("vfs_syscall_readdir enter %p", dp);

    vfs = (vfs_t*)(private);
    vfs_dir = (vfs_dir_t*)(dp);
    rc = vfs->vfs_agentfs.afs_op->readdir(vfs->vfs_agentfs.afs_private,
                    vfs_dir->d_private,
                    (struct dirent **)(&vfs_d_itor));
    if (rc < 0) {
        YSFS_ERROR("readdir failed %d", rc);
        goto l_out;
    }

    *d_itor = vfs_d_itor;
    rc = 0;
    YSFS_TRACE("vfs_syscall_readdir ok %p", dp);

l_out:
    YSFS_TRACE("vfs_syscall_readdir exit %p", dp);
    return rc;
}

static int vfs_syscall_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_rename enter %s-%s", from, to);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->rename(vfs->vfs_agentfs.afs_private, from, to);
    if (rc < 0) {
        YSFS_ERROR("rename failed %d %s-%s" , rc, from, to);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_rename ok %s-%s", from, to);

l_out:
    YSFS_TRACE("vfs_syscall_rename exit %s-%s", from, to);
    return rc;
}

static int vfs_syscall_unlink(void * private, const char *path)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_unlink enter. %s", path);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->unlink(vfs->vfs_agentfs.afs_private, path);
    if (rc < 0) {
        YSFS_ERROR("vfs_syscall_unlink failed %d %s", rc, path);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("vfs_syscall_unlink ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_unlink exit %s", path);
    return rc;
}

static int vfs_syscall_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_file_t * vfs_file = NULL;

    YSFS_TRACE("vfs_syscall_open enter %s", path);

    vfs = (vfs_t*)(private);
    vfs_file = (vfs_file_t*)malloc(sizeof(vfs_file_t));
    if (!vfs_file) {
        rc = -ENOMEM;
         YSFS_ERROR("malloc failed %d %s", rc, path);
        goto l_out;
    }
    rc= vfs->vfs_agentfs.afs_op->open(vfs->vfs_agentfs.afs_private,
                    path, flags, (void **)(&vfs_file->f_private));
    if (rc < 0) {
        YSFS_ERROR("open failed %d %s", rc, path);
        goto l_free;
    }

    rc = 0;
    *file = (void*)vfs_file;
    YSFS_TRACE("vfs_syscall_open ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_open exit %s", path);
    return rc;
l_free:
    free(vfs_file);
    vfs_file = NULL;
    goto l_out;
}

static int vfs_syscall_close(void * private, void * file)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_file_t * vfs_file = NULL;

    YSFS_TRACE("vfs_syscall_close enter");

    vfs = (vfs_t*)(private);
    vfs_file = (vfs_file_t*)(file);
    rc = vfs->vfs_agentfs.afs_op->close(vfs->vfs_agentfs.afs_private,
                    vfs_file->f_private);
    if (rc < 0) {
        YSFS_ERROR("close failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_close ok");

l_out:
    YSFS_TRACE("vfs_syscall_close exit");
    return 0;
}

static ssize_t vfs_syscall_read(void * private, void* file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_file_t * vfs_file = NULL;

    YSFS_TRACE("vfs_syscall_read enter");
    vfs = (vfs_t*)(private);
    vfs_file = (vfs_file_t*)(file);
    rc = vfs->vfs_agentfs.afs_op->read(vfs->vfs_agentfs.afs_private,
                        vfs_file->f_private,
                        buff,
                        buff_size,
                        off);
    if (rc < 0) {
        YSFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_read ok");

l_out:
     YSFS_TRACE("vfs_syscall_read exit");
    return rc;
}

static ssize_t vfs_syscall_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_file_t * vfs_file = NULL;
    
    YSFS_TRACE("vfs_syscall_write enter");

    vfs = (vfs_t*)(private);
    vfs_file = (vfs_file_t*)(file);
    rc = vfs->vfs_agentfs.afs_op->write(vfs->vfs_agentfs.afs_private,
                    vfs_file->f_private,
                    buff, buff_size,
                    off);
    if (rc < 0) {
        YSFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_write ok");

l_out:
    YSFS_TRACE("vfs_syscall_write exit");
    return rc;
}

static int vfs_syscall_fsync(void * private,
                void* file,
                int isdatasync)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_file_t * vfs_file = NULL;

    YSFS_TRACE("vfs_syscall_fsync enter");

    vfs = (vfs_t*)(private);
    vfs_file = (vfs_file_t*)(file);
    rc = vfs->vfs_agentfs.afs_op->fsync(vfs->vfs_agentfs.afs_private,
                    vfs_file->f_private,
                    isdatasync);
    if (rc < 0) {
        YSFS_ERROR("fsync failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_fsync ok");

l_out:
    YSFS_TRACE("vfs_syscall_fsync exit");
    return rc;
}

static int vfs_syscall_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    vfs_file_t * vfs_file = NULL;

    YSFS_TRACE("vfs_syscall_ftruncate enter");

    vfs = (vfs_t*)(private);
    vfs_file = (vfs_file_t*)(file);
    
    rc = vfs->vfs_agentfs.afs_op->ftruncate(vfs->vfs_agentfs.afs_private,
                    vfs_file->f_private,
                    size);
    if (rc < 0) {
        YSFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("vfs_syscall_ftruncate ok");

l_out:
     YSFS_TRACE("vfs_syscall_ftruncate exit");
    return rc;
}

static int vfs_syscall_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_truncate enter %s", path);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->truncate(vfs->vfs_agentfs.afs_private, path, size);
    if (rc < 0) {
        YSFS_ERROR("truncate failed %d, %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_truncate ok");

l_out:
    YSFS_TRACE("vfs_syscall_truncate exit");
    return rc;
}

static int vfs_syscall_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_symlink enter %s-%s", from, to);

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->symlink(vfs->vfs_agentfs.afs_private, from, to);
    if (rc < 0) {
        YSFS_ERROR("symlink failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_symlink ok");

l_out:
    YSFS_TRACE("vfs_syscall_symlink exit");
    return rc;
}
    // unsupported functions
static int vfs_syscall_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_link enter");

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->link(vfs->vfs_agentfs.afs_private, from, to);
    if (rc < 0) {
        YSFS_ERROR("link failed %d, %s", rc, to);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("vfs_syscall_link ok");

l_out:
     YSFS_TRACE("vfs_syscall_link exit");
    return rc;
}

static int vfs_syscall_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc =0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_mknod enter %s", path);
    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->mknod(vfs->vfs_agentfs.afs_private, path, mode, dev);
    if (rc < 0) {
        YSFS_ERROR("mknod failed %d, %s", rc, path);
        goto l_out;
    }
    rc = 0;
    YSFS_TRACE("vfs_syscall_mknod ok  %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_mknod exit %s", path);
    return rc;
}

static int vfs_syscall_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_chmod enter %s", path);
    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->chmod(vfs->vfs_agentfs.afs_private, path, mode);
    if (rc < 0) {
        YSFS_ERROR("vfs_syscall_chmod failed %d, %s", rc, path);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_syscall_chmod ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_chmod exit %s", path);
    return rc;
}

static int vfs_syscall_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_syscall_chown enter %s", path);
    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->chown(vfs->vfs_agentfs.afs_private, path, uid, gid);
    if (rc < 0) {
        YSFS_ERROR("vfs_syscall_chown failed %d, %s", rc, path);
        goto l_out;
    }

    rc= 0;
    YSFS_TRACE("vfs_syscall_chown ok %s", path);

l_out:
    YSFS_TRACE("vfs_syscall_chown exit %s", path);
    return rc;
}

static int vfs_mount(void * private)
{
    int rc = 0;
    vfs_t * vfs = NULL;

    YSFS_TRACE("vfs_mount enter ");

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->mount(vfs->vfs_agentfs.afs_private);
    if (rc < 0) {
        YSFS_ERROR("vfs_mount failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("vfs_mount ok");

l_out:
     YSFS_TRACE("vfs_mount exit");
    return rc;
}

static int vfs_unmount(void * private)
{
    int rc = 0;
    vfs_t * vfs = NULL;
    YSFS_TRACE("vfs_unmount enter");

    vfs = (vfs_t*)(private);
    rc = vfs->vfs_agentfs.afs_op->unmount(vfs->vfs_agentfs.afs_private);
    if (rc < 0) {
        YSFS_ERROR("vfs_unmount failed %d", rc);
        goto l_out;
    }

    rc = 0;
    YSFS_TRACE("passfs_unmount ok");

l_out:
    YSFS_TRACE("passfs_unmount exit");
    return rc;
}


static vfs_syscall_t g_vfs_syscall_op =
{
    .stat = vfs_syscall_stat,
    .getattr = vfs_syscall_getattr,
    .access = vfs_syscall_access,
    .mkdir = vfs_syscall_mkdir,
    .opendir = vfs_syscall_opendir,
    .closedir = vfs_syscall_closedir,
    .readdir = vfs_syscall_readdir,
    .rename = vfs_syscall_rename,
    .unlink = vfs_syscall_unlink,
    .open = vfs_syscall_open,
    .close = vfs_syscall_close,
    .read = vfs_syscall_read,
    .write = vfs_syscall_write,
    .fsync = vfs_syscall_fsync,
    .ftruncate = vfs_syscall_ftruncate,
    .truncate = vfs_syscall_truncate,
    .link = vfs_syscall_link,
    .mknod = vfs_syscall_mknod,
    .chmod = vfs_syscall_chmod,
    .chown= vfs_syscall_chown,
    .mount = vfs_mount,
    .unmount = vfs_unmount,
    .infs = vfs_infs,
};


int vfs_malloc_fs(vfs_config_t * config, vfs_t ** vfs)
{
    int rc = 0;
    vfs_t * entry = NULL;
    passfs_t * passfs = NULL;

    YSFS_TRACE("malloc_vfs enter");
    entry = (vfs_t*)malloc(sizeof(vfs_t));
    if (!entry) {
        rc = -ENOMEM;
        goto l_out;
    }

    rc = passfs_malloc_fs(&passfs);
    if (!passfs) {
        rc = -ENOMEM;
        goto l_free_fs;
    }
    //entry->vfs_block;
    passfs->pfs_config.pfs_bdev = strdup(config->vfs_dev);
    entry->vfs_agentfs.afs_op = passfs->pfs_op;
    entry->vfs_agentfs.afs_private = passfs;
    entry->vfs_op = &g_vfs_syscall_op;
    *vfs = entry;
    rc = 0;
    YSFS_TRACE("malloc_vfs ok");

l_out:
    YSFS_TRACE("malloc_vfs exit");
    return rc;

l_free_fs:
    free(entry);
    goto l_out;
}


void vfs_free_fs(vfs_t * vfs)
{
    passfs_t * passfs = NULL;

    YSFS_TRACE("vfs_free_fs enter");
    passfs = (passfs_t*)vfs->vfs_agentfs.afs_private;
    if (!passfs) {
        return;
    }
    if (passfs->pfs_config.pfs_bdev) {
        free(passfs->pfs_config.pfs_bdev);
        passfs->pfs_config.pfs_bdev = NULL;
    }
    passfs_free_fs(passfs);
    vfs->vfs_agentfs.afs_private = NULL;
     vfs->vfs_agentfs.afs_op = NULL;
    free(vfs);

    YSFS_TRACE("vfs_free_fs exit");
}


