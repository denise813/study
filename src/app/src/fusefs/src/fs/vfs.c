#include <stdlib.h>
#include <string.h>

#include "vfs_op.h"
#include "vfs.h"
#include "src/ysfs_log.h"
#include "src/fs/passfs/passfs.h"


int vfs_malloc_fs(vfs_config_t * config, vfs_t **fs)
{
    vfs_t * entry = NULL;
    passfs_t * passfs = NULL;

    YSFS_TRACE("malloc_vfs enter");
    entry = (vfs_t*)malloc(sizeof(vfs_t));

    passfs_malloc_fs(&passfs);
    //entry->vfs_block;
    passfs->pfs_config.pfs_bdev = strdup(config->vfs_dev);
    entry->vfs_private = passfs;
    entry->vfs_op = passfs->pfs_op;
    *fs = entry;

    YSFS_TRACE("malloc_vfs exit");
    return 0;
}


void vfs_free_fs(vfs_t *fs)
{
    passfs_t * passfs = NULL;

    passfs = (passfs_t*)fs->vfs_private;
    YSFS_TRACE("malloc_vfs enter");
    if (!passfs) {
        return;
    }
    if (passfs->pfs_config.pfs_bdev) {
        free(passfs->pfs_config.pfs_bdev);
    }
    passfs_free_fs(passfs);

    YSFS_TRACE("malloc_vfs exit");

    free(fs);
}


