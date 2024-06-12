#ifndef FUSEFS_STORAGE_H
#define FUSEFS_STORAGE_H


#include "fusefs_config.h"
#include "fusefs_storage_op.h"


#ifdef __cplusplus
extern "C"
{
#endif

#define FUSEFS_STORAGE_BACKEND_TYPE_PASSFS "PASSFS"
#define FUSEFS_STORAGE_BACKEND_TYPE_GUESTFS "GUESTFS"



typedef struct fusefs_storage_config
{
    char * sc_typename;
    int sc_type;
    char ** sc_devs[100];
    int sc_devs_num;
}fusefs_storage_config_t;

typedef struct fusefs_agent_storage
{
    void * as_stroage;
    storage_op_t * as_stroage_op;
}fusefs_agent_storage_t;

typedef struct fusefs_storage
{
    fusefs_storage_config_t s_config;
    fusefs_agent_storage_t s_agent;
    storage_op_t * s_op;
}fusefs_storage_t;

typedef struct fusefs_storage_inode
{
    void * f_private;
}fusefs_storage_info_t;

typedef struct fusefs_storage_file
{
     void * f_private;
}fusefs_storage_file_t;

typedef struct fusefs_storage_dir
{
     void * d_private;
     //struct dirent *d_itor;
     //int d_index;
}fusefs_storage_dir_t;


int fusefs_malloc_storage(fusefs_config_t *config, fusefs_storage_t **fs);
void fusefs_free_storage(fusefs_storage_t *fs);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
