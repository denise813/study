#ifndef FUSEFS_CONFIG_H
#define FUSEFS_CONFIG_H


#ifdef __cplusplus
extern "C"
{
#endif


#define FUSEFS_STORAGE_TYPE_PASSFS "passfs"
#define FUSEFS_STORAGE_TYPE_GUESTFS "guestfs"
#define FUSEFS_STORAGE_TYPE_SPFS "spfs"
enum{
    FUSEFS_STORAGE_BACKEND_PASSFS,
    FUSEFS_STORAGE_BACKEND_GUESTFS,
    FUSEFS_STORAGE_BACKEND_SPFS,
    FUSEFS_STORAGE_BACKEND_UNKOWN,
};

enum{
    FUSEFS_OP_TYPE_MOUNT,
    FUSEFS_OP_TYPE_CREATE,
    FUSEFS_OP_TYPE_UNKOWN,
};


typedef struct fusefs_config
{
    char * fusefs_bdevs[100];
    char * fusefs_mountpoint;
    int fusefs_op_type;
    int fusefs_fs_type;
    int fusefs_bdevs_num;
    int  fusefs_has_help;
}fusefs_config_t;



int fusefs_parse_configure(int, char *[], fusefs_config_t*);
void fusefs_help();
int fusefs_malloc_config(fusefs_config_t**);
void fusefs_free_config(fusefs_config_t*);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
