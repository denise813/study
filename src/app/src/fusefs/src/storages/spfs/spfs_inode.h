// Inode manipulation routines.
//
// Feel free to use as inspiration.

// based on cs3650 starter code

#ifndef SPFS_INODE_H
#define SPFS_INODE_H

#include "spfs_blocks.h"
#include "time.h"

typedef struct spfs_inode {
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block; // single block pointer (if max file size <= 4K)

  time_t atime; // access time
  time_t mtime; // modify time
  time_t ctime; // change time
}spfs_inode_t;


int spfs_get_inode(spfs_inode_t** inode);
int spfs_put_inode(spfs_inode_t* inode);


void spfs_print_inode(spfs_inode_t *node);
spfs_inode_t *spfs_get_inode(int inum);
int spfs_alloc_inode();
void spfs_free_inode(int inum);
int spfs_grow_inode(spfs_inode_t *node, int size);
int spfs_shrink_inode(spfs_inode_t *node, int size);
int spfs_inode_get_pnum(spfs_inode_t *node, int fpn);

#endif
