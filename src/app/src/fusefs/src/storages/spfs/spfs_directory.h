// Directory manipulation functions.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef DIRECTORY_H
#define DIRECTORY_H

#define DIR_NAME_LENGTH 48

#include "spfs_blocks.h"
#include "spfs_inode.h"
#include "spfs_slist.h"

typedef struct spfs_dirent {
  char name[DIR_NAME_LENGTH];
  int inum;
  char _reserved[12];
}spfs_dirent_t;

void spfs_directory_init();
int spfs_directory_lookup(spfs_inode_t *dd, const char *name);
int spfs_tree_lookup(const char *path);
int spfs_directory_put(spfs_inode_t *dd, const char *name, int inum);
int spfs_directory_delete(spfs_inode_t *dd, const char *name);
spfs_slist_t * spfs_directory_list(const char *path);
void spfs_print_directory(spfs_inode_t *dd);

#endif
