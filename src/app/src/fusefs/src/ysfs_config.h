#ifndef YSFS_CONFIG_H
#define YSFS_CONFIG_H


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct ysfs_config {
    char * ysc_dev;
    char * ysc_mount_point;
    char * ysc_fs_name;
    int ysc_has_help;
}ysfs_config_t;


int cmd_parse_configure(int, char *[], ysfs_config_t*);
void cmd_help_configure();
int malloc_ys_config(ysfs_config_t**);
void free_ys_config(ysfs_config_t*);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
