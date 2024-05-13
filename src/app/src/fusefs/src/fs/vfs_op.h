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
    int (*opendir)(void * private, const char *path, DIR **dp);
    int (*closedir)(void * private, DIR *dp);
    int (*readdir)(void * private, DIR *dp,
                    struct dirent **de);
    int (*rename)(void * private, const char *from, const char *to);
    int (*unlink)(void * private, const char *path);
    int (*open)(void * private, const char *path, int flags, int *fd);
    int (*close)(void * private, int fd);
    int (*read)(void * private, int fd, char *buf, size_t buff_size,
                    off_t off);
    int (*write)(void * private,
                    int fd, const char *buf, size_t buff_size,
                    off_t off);
    int (*fsync)(void * private,
                    int fd,
                    int isdatasync);
    int (*ftruncate)(void * private,int fd, off_t size);
    int (*truncate)(void * private,const char *path, off_t size);
    int (*symlink)(void *private, const char *from, const char *to);
    // unsupported functions
    int (*link)(void * private, const char *from, const char *to);
    int (*mknod)(void *private, const char *path, mode_t mode, dev_t dev);
    int (*chmod)(void * private, const char *path, mode_t mode);
    int (*chown)(void * private, const char *path, uid_t uid, gid_t gid);
     int (*mount)(void * private);
    int (*unmount)(void * private);
}vfs_syscall_t;



#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
