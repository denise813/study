#ifndef FUSEFS_CONFIG_H
#define FUSEFS_CONFIG_H


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct fusefs_config
{
    char * fusefs_bdevs[100];
    char * fusefs_mountpoint;
    char * fusefs_fsname;
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
