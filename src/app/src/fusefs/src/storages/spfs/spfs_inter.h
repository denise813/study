#ifndef EXTFS_H
#define EXTFS_H


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct extfs_config
{
    char * extfs_bdev;
}extfs_config_t;


typedef struct extfs
{
    extfs_config_t extfs_config;
    vfs_syscall_t * extfs_op;
}extfs_t;


int extfs_malloc_fs(extfs_t **fs);
void extfs_free_fs(extfs_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
