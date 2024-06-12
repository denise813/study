#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "directory.h"
#include "bitmap.h"
#include "inode.h"
#include "slist.h"

void spfs_directory_init()
{
    spfs_inode_t* root_node = get_inode(alloc_inode());
    root_node->mode = 040755; // directory
}

int spfs_directory_lookup(spfs_inode_t *dd, const char *name)
{
    // root directory, return 0;
    if (!strcmp(name, ""))
        return 0;
    dirent_t* dirs = blocks_get_block(dd->block);
    for (int ii = 0; ii < 64; ii++)
        if (!strcmp(name, dirs[ii].name))
            return dirs[ii].inum;
    return -ENOENT; // No such directory, return -ENOENT
}

int spfs_tree_lookup(const char *path, spfs_inode* inode)
{
    slist_t* path_slist = s_explode(path, '/');
    slist_t* pt_slist = path_slist;
    int inum = 0;
    while (pt_slist)
    {
        inum = spfs_directory_lookup(get_inode(inum), pt_slist->data);
        if (inum < 0)
        {
            s_free(path_slist);
            printf("Debug: tree_lookup(%s), return %d\n", path, inum);
            return -ENOENT; // No such directory, return -ENOENT
        }
        pt_slist = pt_slist->next;
    }
    s_free(path_slist);

    printf("Debug: tree_lookup(%s), return %d\n", path, inum);
    return inum;
}

int spfs_directory_put(spfs_inode_t *dd, const char *name, int inum)
{
    printf("Debug: directory_put(%s, %d)\n", name, inum);
    dirent_t new_dir;
    strncpy(new_dir.name, name, DIR_NAME_LENGTH);
    new_dir.inum = inum;
    int num_dirs = dd->size / sizeof(dirent_t);
    dirent_t* dirs = blocks_get_block(dd->block);
    dirs[num_dirs] = new_dir;
    dd->size += sizeof(dirent_t);
    return 0;
}

int spfs_directory_delete(spfs_inode_t *dd, const char *name)
{
    int num_dirs = dd->size / sizeof(dirent_t);
    dirent_t* dirs = blocks_get_block(dd->block);
    for (int ii = 0; ii < num_dirs; ii++)
        if (!strcmp(dirs[ii].name, name))
        {
            // decrease inode.refs
            spfs_inode_t* node = get_inode(dirs[ii].inum);
            node->refs--;
            if (node->refs <= 0)
                free_inode(dirs[ii].inum);
            // shrink dirs
            for (int jj = ii; jj < num_dirs - 1; jj++)
                dirs[jj] = dirs[jj + 1];
            dd->size -= sizeof(dirent_t);
            return 0;
        }
    return -ENOENT; // No such directory, return -ENOENT
}

spfs_slist_t *spfs_directory_list(const char *path)
{
    int inum = spfs_tree_lookup(path);
    spfs_inode_t* node = spfs_get_inode(inum);
    int num_dirs = node->size / sizeof(dirent_t);
    dirent_t* dirs = spfs_blocks_get_block(node->block);

    slist_t* ret = NULL;
    for (int ii = 0; ii < num_dirs; ii++)
        ret = s_cons(dirs[ii].name, ret);
    return ret;
}

void spfs_print_directory(spfs_inode_t *dd)
{
    int num_dirs = dd->size / sizeof(dirent_t);
    dirent_t* dirs = blocks_get_block(dd->block);
    for (int ii = 0; ii < num_dirs; ii++)
    {
        printf("Dir %d:\n", ii);
        printf("Name: %s\n", dirs[ii].name);
        printf("Inum: %d\n", dirs[ii].inum);
    }
}
