#include <stdio.h>
#include "src/fusefs_log.h"
#include "src/fusefs_config.h"
#include "src/mount/fusefs_fs.h"


static int do_create(fusefs_config_t * p_config)
{
    return 0;
}

static int do_mount(int argc, char *argv[], fusefs_config_t * p_config)
{
    int rc = 0;
    fusefs_fs_t * p_fs = NULL;

    rc = fusefs_malloc_fs(p_config, &p_fs);
    if (rc < 0) {
        goto l_out;
    }

    rc = p_fs->init(p_fs);
    if (rc < 0) {
        goto l_free_fs;
    }

    rc= p_fs->run(argc, argv, p_fs);
    if (rc < 0) {
        goto l_free_fs;
    }

    p_fs->exit(p_fs);
    fusefs_free_fs(p_fs);
    rc = 0;

l_out:
    return rc;

l_free_fs:
    fusefs_free_fs(p_fs);
    goto l_out;
}

int main(int argc, char *argv[])
{
    int rc = 0;
    fusefs_config_t * p_config;
    fusefs_fs_t * p_fs;
    
    rc = fusefs_malloc_config(&p_config);
     if (rc < 0) {
        goto l_out;
    }

    rc = fusefs_parse_configure(argc, argv, p_config);
    if (rc < 0) {
        goto l_free_config;
    }

    if (p_config->fusefs_has_help) {
        rc = 0;
        fusefs_help();
        goto l_free_config;
    }

    if (strcmp(p_config->fusefs_subcmd, FUSEFS_SUBCMD_MOUNT) == 0) {
        rc = do_mount(argc, argv, p_config);
    } else if (strcmp(p_config->fusefs_subcmd == FUSEFS_SUBCMD_CREATE) == 0) {
        rc = do_create(p_config);
    }
    if (rc < 0) {
        goto l_free_config;
    }

    fusefs_free_config(p_config);
    rc = 0;

l_out:
    return rc;

l_free_config:
    fusefs_free_config(p_config);
    p_config = NULL;
    goto l_out;
}
