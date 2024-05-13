#ifndef PASS_H
#define PASS_H


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct passfs
{
    vfs_syscall_t * vfs_op;
}passfs_t;


int malloc_passfs(passfs_t **fs);
void free_passfs(passfs_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
