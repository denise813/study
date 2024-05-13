#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "ysfs_log.h"
#include "ysfs_config.h"

extern int optind, opterr, optopt;


enum
{
    YSFS_YSFS_OPT_VALUE_DEV = 0,
    YSFS_OPT_VALUE_MOUNT_POINT,
    YSFS_OPT_VALUE_FS_NAME,
    YSFS_OPT_VALUE_HELP,
    YSFS_OPT_VALUE_ID_END,
};


typedef struct help_option
{
    int optId;
    char * name;
    char * help;
    int requested;
    int hasflag;
}help_option_t;


const struct option g_options[] = {
                {"dev", required_argument, NULL, YSFS_YSFS_OPT_VALUE_DEV},
                {"mountpoint", required_argument, NULL, YSFS_OPT_VALUE_MOUNT_POINT},
                {"fs", required_argument, NULL, YSFS_OPT_VALUE_FS_NAME},
                {"help", no_argument, NULL, YSFS_OPT_VALUE_HELP},
                {NULL, 0, NULL, 0}
                };


help_option_t g_option_helps[] = {
                {YSFS_YSFS_OPT_VALUE_DEV, "dev", "--dev /dev/sdb", TRUE, FALSE},
                {YSFS_OPT_VALUE_MOUNT_POINT, "mountpoint", "--mountpoint /mnt/tst", TRUE, FALSE},
                {YSFS_OPT_VALUE_FS_NAME, "fs", "--fs ext2", TRUE, FALSE},
                {YSFS_OPT_VALUE_HELP, "help", "--help.", FALSE, FALSE},
                {0, "", "", 0, 0}
                };


int cmd_parse_configure(int argc, char *argv[], ysfs_config_t * config)
{
    int rc = 0;
    int option_index = 0;
    int opt_value = 0;
    char * cmd = argv[0];
    char * context = NULL;

    if (argc < 1) {
        rc = -EINVAL;
        YSFS_ERROR("parse failed, rc=(%d).", rc);
        return rc;
    }

    YSFS_TRACE("parse start.");

    const char * optstring = "";
    while(opt_value != -1) {
        opt_value = getopt_long(argc, argv, optstring, g_options, &option_index);
        if (opt_value == -1) {
            break;
        }

        switch(opt_value) {
            case YSFS_YSFS_OPT_VALUE_DEV:
                context = optarg;
                g_option_helps[option_index].hasflag = TRUE;
                config->ysc_dev = strdup(context);
                YSFS_INFO("configure parse dev=%s, mandatory=%d.",
                                context,
                                g_option_helps[option_index].requested);
                break;
            case YSFS_OPT_VALUE_MOUNT_POINT:
                context =  optarg;
                config->ysc_mount_point = strdup(context);
                g_option_helps[option_index].hasflag = TRUE;
                YSFS_INFO("configure parse moutpoint=%s, mandatory=%d.",
                                context,
                                g_option_helps[option_index].requested);
                break;
            case YSFS_OPT_VALUE_FS_NAME:
                context =  optarg;
                config->ysc_fs_name = strdup(context);
                g_option_helps[option_index].hasflag = TRUE;
                YSFS_INFO("configure parse fs=%s, mandatory=%d.",
                                context,
                                g_option_helps[option_index].requested);
                break;
            case YSFS_OPT_VALUE_HELP:
                g_option_helps[option_index].hasflag = TRUE;
                config->ysc_has_help = 1;
                YSFS_INFO("configure help.");
                return 0;
            default:
                 YSFS_ERROR("has invalid option.");
                return -1;
        }
    }

    YSFS_INFO("\ndev=%s \n" "mountpoint=%s \n" "fs=%s \n",
                    config->ysc_dev,
                    config->ysc_mount_point,
                    config->ysc_fs_name);

    int opt_num = (sizeof(g_option_helps) / sizeof(help_option_t) - 1);
    for (int i = 0; i < opt_num; i++) {
        if (g_option_helps[i].requested == TRUE) {
            if (g_option_helps[i].hasflag == FALSE) {
                printf("miss args %s\n", g_option_helps[i].name);
                YSFS_ERROR("miss args %s", g_option_helps[i].name);
                return -EINVAL;
            }
        }
    }

    rc = 0;
    YSFS_TRACE("parse exit.");
    return rc;
}

void cmd_help_configure()
{
    int i = 0;
    int opt_num = (sizeof(g_option_helps) / sizeof(help_option_t) - 1);

     printf("args: \n");
    for (i = 0; i < opt_num; i++) {
        if (g_option_helps[i].requested == TRUE){
            printf("\t%s\t\t mandatory\t\t %s \n", g_option_helps[i].name, g_option_helps[i].help);
        } else {
            printf("\t%s\t\t option\t\t %s \n", g_option_helps[i].name, g_option_helps[i].help);
        }
    }
    return;
}

int malloc_ys_config(ysfs_config_t ** config)
{
    int rc = 0;
    ysfs_config_t * entry = NULL;

    entry = (ysfs_config_t*)malloc(sizeof(ysfs_config_t));
    entry->ysc_has_help = 0;
    entry->ysc_dev = NULL;
    entry->ysc_fs_name = NULL;
    entry->ysc_mount_point = NULL;
    *config = entry;

    return rc;
}
void free_ys_config(ysfs_config_t * config)
{
    config->ysc_has_help = 0;
    if (config->ysc_dev) {
        free(config->ysc_dev);
        config->ysc_dev = NULL;
    }
    if (config->ysc_fs_name) {
        free(config->ysc_fs_name);
        config->ysc_fs_name = NULL;
    }
    if (config->ysc_mount_point) {
        free(config->ysc_mount_point);
        config->ysc_mount_point = NULL;
    }
    free(config);
}


