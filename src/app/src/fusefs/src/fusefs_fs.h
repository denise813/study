#ifndef FUSEFS_FS_H
#define FUSEFS_FS_H

#include "fusefs_fuse_def.h"
#include "fusefs_config.h"
#include "fusefs_storage.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct fusefs_fs
{
    fusefs_config_t fusefs_fs_config;
    struct fuse_operations * fusefs_fs_op;
    fusefs_storage_t * fusefs_storage;
    int (*init)(struct fusefs_fs*);
    int (*run)(int, char *[], struct fusefs_fs*);
    int (*exit)(struct fusefs_fs*);
}fusefs_fs_t;


int fusefs_malloc_fs(fusefs_config_t *, fusefs_fs_t**);
void fusefs_free_fs(fusefs_fs_t *);
fusefs_fs_t* fusefs_get_fs();
struct fuse_operations * fusefs_get_fs_op();


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
