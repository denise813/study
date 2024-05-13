#include <stdlib.h>

#include "vfs_op.h"
#include "vfs.h"
#include "src/fs/passfs/passfs.h"


int malloc_vfs(char * fs_name, vfs_t **fs)
{
    vfs_t * entry = NULL;
    passfs_t * passfs = NULL;

    entry = (vfs_t*)malloc(sizeof(vfs_t));

    malloc_passfs(&passfs);
    //entry->vfs_block;
    entry->vfs_private = passfs;
    entry->vfs_op = passfs->vfs_op;
    *fs = entry;
    return 0;
}

void free_vfs(vfs_t *fs)
{
    passfs_t * passfs = (passfs_t*)fs->vfs_private;
    free_passfs(passfs);

    free(fs);
}

