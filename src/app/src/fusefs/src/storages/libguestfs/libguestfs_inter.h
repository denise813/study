#ifndef PASS_H
#define PASS_H


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
    vfs_syscall_t * pfs_op;
}passfs_t;


int passfs_malloc_fs(passfs_t **fs);
void passfs_free_fs(passfs_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
