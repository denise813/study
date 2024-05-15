#include <stdlib.h>

#include "vfs_op.h"
#include "vfs.h"
#include "src/ysfs_log.h"
#include "src/fs/passfs/passfs.h"


int vfs_malloc_fs(char * fs_name, vfs_t **fs)
{
    vfs_t * entry = NULL;
    passfs_t * passfs = NULL;

    YSFS_TRACE("malloc_vfs enter");
    entry = (vfs_t*)malloc(sizeof(vfs_t));

    passfs_malloc_fs(&passfs);
    //entry->vfs_block;
    entry->vfs_private = passfs;
    entry->vfs_op = passfs->vfs_op;
    *fs = entry;

    YSFS_TRACE("malloc_vfs exit");
    return 0;
}

void vfs_free_fs(vfs_t *fs)
{
    passfs_t * passfs = NULL;

    passfs = (passfs_t*)fs->vfs_private;
    YSFS_TRACE("malloc_vfs enter");
    
    passfs_free_fs(passfs);

    YSFS_TRACE("malloc_vfs exit");

    free(fs);
}

