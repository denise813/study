#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "fusefs_storage_op.h"
#include "fusefs_storage.h"
#include "fusefs_log.h"
#include "fusefs_config.h"
#include "storages/passfs/passfs_inter.h"
#ifdef STORAGE_ENABLE_BACKEND_GUESTFS
#include "storages/libguestfs/libguestfs_inter.h"
#endif

static int fuse_storage_init(void * private)
{
    return 0;
}

static int fuse_storage_exit(void * private)
{
    return 0;
}

static int fuse_storage_mount(void * private)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_mount enter ");

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->mount(fuse_storage->s_agent.as_stroage);
    if (rc < 0) {
        FUSEFS_ERROR("fuse_storage_mount failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_mount ok");

l_out:
     FUSEFS_TRACE("fuse_storage_mount exit");
    return rc;
}

static int fuse_storage_umount(void * private)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    FUSEFS_TRACE("fuse_storage_unmount enter");

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->umount(fuse_storage->s_agent.as_stroage);
    if (rc < 0) {
        FUSEFS_ERROR("fuse_storage_unmount failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_unmount ok");

l_out:
    FUSEFS_TRACE("passfs_unmount exit");
    return rc;
}


static int fuse_storage_infs(void * private,const char * path)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_infs enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
     rc = fuse_storage->s_agent.as_stroage_op->infs(fuse_storage->s_agent.as_stroage, path);
    if (rc < 0) {
        FUSEFS_ERROR("infs failed, errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_infs ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_infs exit %s", path);
    return 0;
}

static int fuse_storage_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_stat enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->stat(fuse_storage->s_agent.as_stroage, path, s);
    if (rc < 0) {
        FUSEFS_ERROR("stat failed, errno(%d) %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fuse_storage_stat ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_stat exit %s", path);
    return rc;
}

static int fuse_storage_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_getattr enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->getattr(fuse_storage->s_agent.as_stroage, path, s);
    if (rc < 0) {
        FUSEFS_ERROR("getattr failed, errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_getattr ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_getattr exit %s", path);
    return rc;
}

static int fuse_storage_access(void * private, const char *path, int mask)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("passfs_access enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->access(fuse_storage->s_agent.as_stroage, path, mask);
    if (rc < 0) {
        FUSEFS_ERROR("access failed, errno(%d) %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_access ok %s", path);

l_out:
    FUSEFS_TRACE("passfs_access exit %s", path);
    return 0;
}

static int fuse_storage_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
 
    FUSEFS_TRACE("fuse_storage_mkdir enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->mkdir(fuse_storage->s_agent.as_stroage, path, mode);
     if (rc < 0) {
        FUSEFS_ERROR("mkdir failed, errno(%d) %s", rc, path);
        goto l_out;
    }
     
    rc = 0;
    FUSEFS_TRACE("fuse_storage_mkdir ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_mkdir exit %s", path);
    return rc;
}

static int fuse_storage_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_dir_t * fuse_storage_dir = NULL;

    FUSEFS_TRACE("fuse_storage_opendir enter %s", path);

    fuse_storage_dir = (fusefs_storage_dir_t*)malloc(sizeof(fusefs_storage_dir_t));
    if (!fuse_storage_dir) {
        rc = -ENOMEM;
        FUSEFS_ERROR("malloc failed, errno(%d) %s", rc, path);
        goto l_out;
    }
    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->opendir(fuse_storage->s_agent.as_stroage,
                    path,
                    &fuse_storage_dir->d_private);
    if (rc < 0) {
        FUSEFS_ERROR("opendir failed %d %s", rc, path);
        rc = -1;
        goto l_free;
    }

    *dp = (void*)fuse_storage_dir;
    rc = 0;
    FUSEFS_TRACE("fuse_storage_opendir ok %s %p", path, *dp);

l_out:
    FUSEFS_TRACE("fuse_storage_opendir exit %s %p", path, *dp);
    return rc;
l_free:
    free(fuse_storage_dir);
    fuse_storage_dir = NULL;
    goto l_out;
}

static int fuse_storage_closedir(void * private, void *dp)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_dir_t * fuse_storage_dir = NULL;

    FUSEFS_TRACE("passfs_closedir enter %p", dp);

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_dir = (fusefs_storage_dir_t*)(dp);
    rc = fuse_storage->s_agent.as_stroage_op->closedir(fuse_storage->s_agent.as_stroage,
                    fuse_storage_dir->d_private);
     if (rc < 0) {
        FUSEFS_ERROR("closedir failed %d %p", rc, dp);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("passfs_closedir ok %p", dp);

l_out:
    FUSEFS_TRACE("passfs_closedir exit %p", dp);
    return rc;
}

static int fuse_storage_readdir(void * private, void *dp, struct dirent ** d_itor)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_dir_t * fuse_storage_dir = NULL;
    struct dirent * fuse_storage_d_itor = NULL;

    FUSEFS_TRACE("fuse_storage_readdir enter %p", dp);

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_dir = (fusefs_storage_dir_t*)(dp);
    rc = fuse_storage->s_agent.as_stroage_op->readdir(fuse_storage->s_agent.as_stroage,
                    fuse_storage_dir->d_private,
                    (struct dirent **)(&fuse_storage_d_itor));
    if (rc < 0) {
        FUSEFS_ERROR("readdir failed %d", rc);
        goto l_out;
    }

    *d_itor = fuse_storage_d_itor;
    rc = 0;
    FUSEFS_TRACE("fuse_storage_readdir ok %p", dp);

l_out:
    FUSEFS_TRACE("fuse_storage_readdir exit %p", dp);
    return rc;
}

static int fuse_storage_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_rename enter %s-%s", from, to);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->rename(fuse_storage->s_agent.as_stroage, from, to);
    if (rc < 0) {
        FUSEFS_ERROR("rename failed %d %s-%s" , rc, from, to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_rename ok %s-%s", from, to);

l_out:
    FUSEFS_TRACE("fuse_storage_rename exit %s-%s", from, to);
    return rc;
}

static int fuse_storage_unlink(void * private, const char *path)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_unlink enter. %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->unlink(fuse_storage->s_agent.as_stroage, path);
    if (rc < 0) {
        FUSEFS_ERROR("fuse_storage_unlink failed %d %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fuse_storage_unlink ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_unlink exit %s", path);
    return rc;
}

static int fuse_storage_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_file_t * fuse_storage_file = NULL;

    FUSEFS_TRACE("fuse_storage_open enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_file = (fusefs_storage_file_t*)malloc(sizeof(fusefs_storage_file_t));
    if (!fuse_storage_file) {
        rc = -ENOMEM;
         FUSEFS_ERROR("malloc failed %d %s", rc, path);
        goto l_out;
    }
    rc= fuse_storage->s_agent.as_stroage_op->open(fuse_storage->s_agent.as_stroage,
                    path, flags, (void **)(&fuse_storage_file->f_private));
    if (rc < 0) {
        FUSEFS_ERROR("open failed %d %s", rc, path);
        goto l_free;
    }

    rc = 0;
    *file = (void*)fuse_storage_file;
    FUSEFS_TRACE("fuse_storage_open ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_open exit %s", path);
    return rc;
l_free:
    free(fuse_storage_file);
    fuse_storage_file = NULL;
    goto l_out;
}

static int fuse_storage_close(void * private, void * file)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_file_t * fuse_storage_file = NULL;

    FUSEFS_TRACE("fuse_storage_close enter");

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_file = (fusefs_storage_file_t*)(file);
    rc = fuse_storage->s_agent.as_stroage_op->close(fuse_storage->s_agent.as_stroage,
                    fuse_storage_file->f_private);
    if (rc < 0) {
        FUSEFS_ERROR("close failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_close ok");

l_out:
    FUSEFS_TRACE("fuse_storage_close exit");
    return 0;
}

static ssize_t fuse_storage_read(void * private, void* file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_file_t * fuse_storage_file = NULL;

    FUSEFS_TRACE("fuse_storage_read enter");
    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_file = (fusefs_storage_file_t*)(file);
    rc = fuse_storage->s_agent.as_stroage_op->read(fuse_storage->s_agent.as_stroage,
                        fuse_storage_file->f_private,
                        buff,
                        buff_size,
                        off);
    if (rc < 0) {
        FUSEFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_read ok");

l_out:
     FUSEFS_TRACE("fuse_storage_read exit");
    return rc;
}

static ssize_t fuse_storage_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_file_t * fuse_storage_file = NULL;
    
    FUSEFS_TRACE("fuse_storage_write enter");

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_file = (fusefs_storage_file_t*)(file);
    rc = fuse_storage->s_agent.as_stroage_op->write(fuse_storage->s_agent.as_stroage,
                    fuse_storage_file->f_private,
                    buff, buff_size,
                    off);
    if (rc < 0) {
        FUSEFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_write ok");

l_out:
    FUSEFS_TRACE("fuse_storage_write exit");
    return rc;
}

static int fuse_storage_fsync(void * private,
                void* file,
                int isdatasync)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_file_t * fuse_storage_file = NULL;

    FUSEFS_TRACE("fuse_storage_fsync enter");

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_file = (fusefs_storage_file_t*)(file);
    rc = fuse_storage->s_agent.as_stroage_op->fsync(fuse_storage->s_agent.as_stroage,
                    fuse_storage_file->f_private,
                    isdatasync);
    if (rc < 0) {
        FUSEFS_ERROR("fsync failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_fsync ok");

l_out:
    FUSEFS_TRACE("fuse_storage_fsync exit");
    return rc;
}

static int fuse_storage_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;
    fusefs_storage_file_t * fuse_storage_file = NULL;

    FUSEFS_TRACE("fuse_storage_ftruncate enter");

    fuse_storage = (fusefs_storage_t*)(private);
    fuse_storage_file = (fusefs_storage_file_t*)(file);
    
    rc = fuse_storage->s_agent.as_stroage_op->ftruncate(fuse_storage->s_agent.as_stroage,
                    fuse_storage_file->f_private,
                    size);
    if (rc < 0) {
        FUSEFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fuse_storage_ftruncate ok");

l_out:
     FUSEFS_TRACE("fuse_storage_ftruncate exit");
    return rc;
}

static int fuse_storage_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_truncate enter %s", path);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->truncate(fuse_storage->s_agent.as_stroage, path, size);
    if (rc < 0) {
        FUSEFS_ERROR("truncate failed %d, %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_truncate ok");

l_out:
    FUSEFS_TRACE("fuse_storage_truncate exit");
    return rc;
}

static int fuse_storage_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_symlink enter %s-%s", from, to);

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->symlink(fuse_storage->s_agent.as_stroage, from, to);
    if (rc < 0) {
        FUSEFS_ERROR("symlink failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_symlink ok");

l_out:
    FUSEFS_TRACE("fuse_storage_symlink exit");
    return rc;
}
    // unsupported functions
static int fuse_storage_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_link enter");

    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->link(fuse_storage->s_agent.as_stroage, from, to);
    if (rc < 0) {
        FUSEFS_ERROR("link failed %d, %s", rc, to);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fuse_storage_link ok");

l_out:
     FUSEFS_TRACE("fuse_storage_link exit");
    return rc;
}

static int fuse_storage_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc =0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_mknod enter %s", path);
    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->mknod(fuse_storage->s_agent.as_stroage, path, mode, dev);
    if (rc < 0) {
        FUSEFS_ERROR("mknod failed %d, %s", rc, path);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("fuse_storage_mknod ok  %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_mknod exit %s", path);
    return rc;
}

static int fuse_storage_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_chmod enter %s", path);
    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->chmod(fuse_storage->s_agent.as_stroage, path, mode);
    if (rc < 0) {
        FUSEFS_ERROR("fuse_storage_chmod failed %d, %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("fuse_storage_chmod ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_chmod exit %s", path);
    return rc;
}

static int fuse_storage_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    fusefs_storage_t * fuse_storage = NULL;

    FUSEFS_TRACE("fuse_storage_chown enter %s", path);
    fuse_storage = (fusefs_storage_t*)(private);
    rc = fuse_storage->s_agent.as_stroage_op->chown(fuse_storage->s_agent.as_stroage, path, uid, gid);
    if (rc < 0) {
        FUSEFS_ERROR("fuse_storage_chown failed %d, %s", rc, path);
        goto l_out;
    }

    rc= 0;
    FUSEFS_TRACE("fuse_storage_chown ok %s", path);

l_out:
    FUSEFS_TRACE("fuse_storage_chown exit %s", path);
    return rc;
}

static storage_op_t g_fuse_storage_op =
{
    .init = fuse_storage_init,
    .exit = fuse_storage_exit,
    .stat = fuse_storage_stat,
    .getattr = fuse_storage_getattr,
    .access = fuse_storage_access,
    .mkdir = fuse_storage_mkdir,
    .opendir = fuse_storage_opendir,
    .closedir = fuse_storage_closedir,
    .readdir = fuse_storage_readdir,
    .rename = fuse_storage_rename,
    .unlink = fuse_storage_unlink,
    .open = fuse_storage_open,
    .close = fuse_storage_close,
    .read = fuse_storage_read,
    .write = fuse_storage_write,
    .fsync = fuse_storage_fsync,
    .ftruncate = fuse_storage_ftruncate,
    .truncate = fuse_storage_truncate,
    .link = fuse_storage_link,
    .mknod = fuse_storage_mknod,
    .chmod = fuse_storage_chmod,
    .chown= fuse_storage_chown,
    .mount = fuse_storage_mount,
    .umount = fuse_storage_umount,
    .infs = fuse_storage_infs,
};


int fusefs_malloc_storage(fusefs_config_t * config, fusefs_storage_t ** storage)
{
    int rc = 0;
    fusefs_storage_t * entry = NULL;
    char * itor_dev = NULL;
    char * dest_dev = NULL;
    int index = 0;

    FUSEFS_TRACE("fusefs_malloc_storage enter");
    entry = (fusefs_storage_t*)malloc(sizeof(fusefs_storage_t));
    if (!entry) {
        rc = -ENOMEM;
        goto l_out;
    }

    for (index = 0; index < config->fusefs_bdevs_num; index++) {
        itor_dev = (char*)config->fusefs_bdevs[index];
        dest_dev = (char*)entry->s_config.sc_devs[index];
        dest_dev = strdup(itor_dev);
    }
    entry->s_config.sc_devs_num = config->fusefs_bdevs_num;
    entry->s_config.sc_devs[entry->s_config.sc_devs_num] = NULL;


    if (config->fusefs_fsname == FUSEFS_STORAGE_BACKEND_TYPE_PASSFS) {
        entry->s_config.sc_type = 1;
        rc = passfs_malloc_fs(entry);
#ifdef STORAGE_ENABLE_BACKEND_GUESTFS
    } else if (config->fusefs_fsname == FUSEFS_STORAGE_BACKEND_TYPE_GUESTFS){
         entry->s_config.sc_type = 2;
         rc = libguestfs_malloc_fs(entry);
#endif
    } else {
        rc = -EINVAL;
        goto l_free_storage;
    }
    if (!entry->s_agent.as_stroage) {
        rc = -ENOMEM;
        goto l_free_storage;
    }

    entry->s_op = &g_fuse_storage_op;
    *storage = entry;
    rc = 0;
    FUSEFS_TRACE("fusefs_malloc_storage ok");

l_out:
    FUSEFS_TRACE("fusefs_malloc_storage exit");
    return rc;

l_free_storage:
    free(entry);
    goto l_out;
}


void fusefs_free_storage(fusefs_storage_t * storage)
{

    FUSEFS_TRACE("fusefs_free_storage enter");

#if 1
    storage->s_config.sc_type = 1;
    passfs_free_fs(storage);
#endif
    storage->s_agent.as_stroage = NULL;
    storage->s_agent.as_stroage_op = NULL;
    free(storage);

    FUSEFS_TRACE("fusefs_free_storage exit");
}


