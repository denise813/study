#ifndef PASSFS_INTER_H
#define PASSFS_INTER_H


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct passfs_config
{
    char * pfs_bdev;
}passfs_config_t;


typedef struct passfs
{
    passfs_config_t pfs_config;
    storage_op_t * pfs_op;
}passfs_t;


int passfs_malloc_fs(void *fuse_storage);
void passfs_free_fs(void *fuse_storage);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
