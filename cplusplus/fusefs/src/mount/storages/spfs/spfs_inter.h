#ifndef SPFS_INTER_H
#define SPFS_INTER_H


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct spfs_config
{
    char * spfs_bdev;
}spfs_config_t;


typedef struct spfs
{
    spfs_config_t spfs_config;
    storage_op_t * spfs_op;
}spfs_t;

typedef struct spfs_dir
{
    char * d_path;
    slist_t * d_slist;
    slist_t * d_index;
    struct dirent * d_itor;
}spfs_dir_t;

typedef struct spfs_file
{
    char * f_path;
}spfs_file_t;


int spfs_malloc_fs(void * fuse_storage);
void spfs_free_fs(void * fuse_storage);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
