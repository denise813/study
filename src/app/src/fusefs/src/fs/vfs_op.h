#ifndef VFS_OP_H
#define VFS_OP_H


#include <dirent.h>
#include <fcntl.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/uio.h>

#include "src/block/vblock.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct vfs_syscall
{
    int (*stat)(void * private, const char *path, struct statvfs *s);
    int(*getattr)(void * private, const char *path, struct stat *s);
    int(*access)(void * private, const char *path, int mask);
    int (*mkdir)(void * private, const char *path, mode_t mode);
    int (*opendir)(void * private, const char *path, void **dp);
    int (*closedir)(void * private, void *dp);
    int (*readdir)(void * private, void *dp, struct dirent **de);
    int (*rename)(void * private, const char *from, const char *to);
    int (*unlink)(void * private, const char *path);
    int (*open)(void * private, const char *path, int flags, void **file);
    int (*close)(void * private, void *file);
    ssize_t (*read)(void * private, void *file,
                    char *buf,
                    size_t buff_size,
                    off_t off);
    ssize_t (*write)(void * private,
                    void *file,
                    const char *buf,
                    size_t buff_size,
                    off_t off);
    int (*fsync)(void * private,
                    void *file,
                    int isdatasync);
    int (*ftruncate)(void * private,void * file, off_t size);
    int (*truncate)(void * private,const char *path, off_t size);
    int (*symlink)(void *private, const char *from, const char *to);
    // unsupported functions
    int (*link)(void * private, const char *from, const char *to);
    int (*mknod)(void *private, const char *path, mode_t mode, dev_t dev);
    int (*chmod)(void * private, const char *path, mode_t mode);
    int (*chown)(void * private, const char *path, uid_t uid, gid_t gid);
    int (*mount)(void * private);
    int (*unmount)(void * private);
    int (*infs)(void * private, const char *path);
}vfs_syscall_t;



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
