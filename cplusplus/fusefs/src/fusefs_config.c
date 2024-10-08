#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "fusefs_log.h"
#include "fusefs_config.h"

extern int optind, opterr, optopt;


#define FUSEFS_STORAGE_TYPE_PASSFS_STR "passfs"
#define FUSEFS_STORAGE_TYPE_GUESTFS_STR "guestfs"
#define FUSEFS_STORAGE_TYPE_SPFS_STR "spfs"

#define FUSEFS_SUBCMD_MOUNT_STR "mount"
#define FUSEFS_SUBCMD_CREATE_STR "create"


enum
{
    FUSEFS_OPT_VALUE_OP = 0,
    FUSEFS_OPT_VALUE_DEV,
    FUSEFS_OPT_VALUE_MOUNT_POINT,
    FUSEFS_OPT_VALUE_FS_NAME,
    FUSEFS_OPT_VALUE_HELP,
    FUSEFS_OPT_VALUE_ID_END,
};


typedef struct help_option
{
    int optId;
    std::string name;
    std::string help;
    int requested;
    int hasflag;
}help_option_t;


const struct option g_options[] = {
                {"op", required_argument, NULL, FUSEFS_OPT_VALUE_OP},
                {"dev", required_argument, NULL, FUSEFS_OPT_VALUE_DEV},
                {"mountpoint", required_argument, NULL, FUSEFS_OPT_VALUE_MOUNT_POINT},
                {"fs", required_argument, NULL, FUSEFS_OPT_VALUE_FS_NAME},
                {"help", no_argument, NULL, FUSEFS_OPT_VALUE_HELP},
                {NULL, 0, NULL, 0}
                };


help_option_t g_option_helps[] = {
                {FUSEFS_OPT_VALUE_OP, "op", "--op create or mount", TRUE, FALSE},
                {FUSEFS_OPT_VALUE_DEV, "dev", "--dev /dev/sdb", TRUE, FALSE},
                {FUSEFS_OPT_VALUE_MOUNT_POINT, "mountpoint", "--mountpoint /mnt/tst", TRUE, FALSE},
                {FUSEFS_OPT_VALUE_FS_NAME, "fs", "--fs passfs or guestfs", FALSE, FALSE},
                {FUSEFS_OPT_VALUE_HELP, "help", "--help.", FALSE, FALSE},
                {0, "", "", 0, 0}
                };


static int fusefs_vaild_fs(char * fsname, int * fs_type)
{
    int rc = 0;
    if (strcmp(fsname, FUSEFS_STORAGE_TYPE_PASSFS_STR) == 0) {
        *fs_type = FUSEFS_STORAGE_BACKEND_PASSFS;
        rc = 0;
        goto l_out;
    }
#ifdef STORAGE_ENABLE_BACKEND_GUESTFS
    if (strcmp(fsname, FUSEFS_STORAGE_TYPE_GUESTFS_STR) == 0) {
        *fs_type = FUSEFS_STORAGE_BACKEND_GUESTFS;
        rc = 0;
        goto l_out;
    }
#endif
    *fs_type = FUSEFS_STORAGE_BACKEND_UNKOWN;
    rc = -1;
l_out:
    return rc;
}

static int fusefs_vaild_op(char * op, int * op_type)
{
    int rc = 0;
    if (strcmp(op, FUSEFS_SUBCMD_MOUNT_STR) == 0) {
        *op_type = FUSEFS_OP_TYPE_MOUNT;
        rc = 0;
        goto l_out;
    }

    if (strcmp(op, FUSEFS_SUBCMD_CREATE_STR) == 0) {
        *op_type = FUSEFS_OP_TYPE_CREATE;
        rc = 0;
        goto l_out;
    }

    *op_type = FUSEFS_OP_TYPE_UNKOWN;
    rc = -1;
l_out:
    return rc;
}

int fusefs_parse_configure(int argc, char *argv[], fusefs_config_t * config)
{
    int rc = 0;
    int opt_value = 0;
    char * cmd = argv[0];
    char * context = NULL;
    int option_index = 0;
    int i = 0;

    if (argc < 1) {
        rc = -EINVAL;
        FUSEFS_ERROR("parse failed, rc=(%d).", rc);
        return rc;
    }

    FUSEFS_TRACE("parse start.");

    const char * optstring = "";
    while(opt_value != -1) {
        opt_value = getopt_long(argc, argv, optstring, g_options, &option_index);
        if (opt_value == -1) {
            break;
        }

        switch(opt_value) {
            case FUSEFS_OPT_VALUE_OP:
                context = optarg;
                g_option_helps[FUSEFS_OPT_VALUE_OP].hasflag = TRUE;
                rc = fusefs_vaild_op(context, &config->fusefs_op_type);
                if (rc < 0) {
                    FUSEFS_ERROR("has invalid op %s %d. must be create or mount",
                                    config->fusefs_op_type, context);
                    return -1;
                }
                FUSEFS_INFO("configure parse op=%s, %d mandatory=%d.",
                                context,
                                config->fusefs_op_type,
                                g_option_helps[FUSEFS_OPT_VALUE_OP].requested);
                break;
            case FUSEFS_OPT_VALUE_DEV:
                context = optarg;
                g_option_helps[FUSEFS_OPT_VALUE_DEV].hasflag = TRUE;
                config->fusefs_bdevs[config->fusefs_bdevs_num] = strdup(context);
                config->fusefs_bdevs_num++;
                FUSEFS_INFO("configure parse dev=%s, mandatory=%d index %d.",
                                context,
                                g_option_helps[FUSEFS_OPT_VALUE_DEV].requested,
                                config->fusefs_bdevs_num);
                break;
            case FUSEFS_OPT_VALUE_MOUNT_POINT:
                context =  optarg;
                config->fusefs_mountpoint = strdup(context);
                g_option_helps[FUSEFS_OPT_VALUE_MOUNT_POINT].hasflag = TRUE;
                FUSEFS_INFO("configure parse moutpoint=%s, mandatory=%d.",
                                context,
                                g_option_helps[FUSEFS_OPT_VALUE_MOUNT_POINT].requested);
                break;
            case FUSEFS_OPT_VALUE_FS_NAME:
                context =  optarg;
                rc = fusefs_vaild_fs(context, &config->fusefs_fs_type);
                if (rc < 0) {
                    FUSEFS_ERROR("has invalid fsname %s %d. must be passfs or guestfs",
                                    config->fusefs_fs_type, context);
                    return -1;
                }
                g_option_helps[FUSEFS_OPT_VALUE_FS_NAME].hasflag = TRUE;
                FUSEFS_INFO("configure parse fs=%s %d, mandatory=%d.",
                                context,
                                config->fusefs_fs_type,
                                g_option_helps[FUSEFS_OPT_VALUE_FS_NAME].requested);
                break;
            case FUSEFS_OPT_VALUE_HELP:
                g_option_helps[FUSEFS_OPT_VALUE_HELP].hasflag = TRUE;
                config->fusefs_has_help = 1;
                FUSEFS_INFO("configure help.");
                return 0;
            default:
                 FUSEFS_ERROR("has invalid option.");
                return -1;
        }
    }

    int opt_num = (sizeof(g_option_helps) / sizeof(help_option_t) - 1);
    for (i = 0; i < opt_num; i++) {
        if (g_option_helps[i].requested == TRUE) {
            if (g_option_helps[i].hasflag == FALSE) {
                printf("miss args %s\n", g_option_helps[i].name);
                FUSEFS_ERROR("miss args %s", g_option_helps[i].name);
                return -EINVAL;
            }
        }
    }
    config->fusefs_bdevs[config->fusefs_bdevs_num] = NULL;

    rc = 0;
l_out:
    FUSEFS_TRACE("parse exit.");
    return rc;
}

void fusefs_help()
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

int fusefs_malloc_config(fusefs_config_t ** config)
{
    int rc = 0;
    int index = 0;
    fusefs_config_t * entry = NULL;

    entry = (fusefs_config_t*)malloc(sizeof(fusefs_config_t));
    if (!entry) {
        rc = -ENOMEM;
        goto l_out;
    }
    entry->fusefs_bdevs_num = 0;
    entry->fusefs_has_help = 0;
    for (index=0; index < 100; index++) {
        entry->fusefs_bdevs[index] = NULL;
    }
    entry->fusefs_fs_type = FUSEFS_STORAGE_BACKEND_UNKOWN;
    entry->fusefs_mountpoint = NULL;
    *config = entry;

l_out:
    return rc;
}

void fusefs_free_config(fusefs_config_t * config)
{
    int index = 0;

    config->fusefs_has_help = 0;
    for (index = 0; index < config->fusefs_bdevs_num; index++) {
        if (!config->fusefs_bdevs[index]){
            continue;
        }
        free(config->fusefs_bdevs[index]);
        config->fusefs_bdevs[index] = NULL;
    }

    if (config->fusefs_mountpoint) {
        free(config->fusefs_mountpoint);
        config->fusefs_mountpoint = NULL;
    }
    free(config);
}

