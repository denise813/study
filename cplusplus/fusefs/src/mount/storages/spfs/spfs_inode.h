#ifndef SPFS_INODE_H
#define SPFS_INODE_H

#include "spfs_blocks.h"
#include "time.h"

typedef struct spfs_op
{
    int (*get_inode)(spfs_inode_t** inode);
    int (*put_inode)(spfs_inode_t* inode);
    void (*print_inode)(spfs_inode_t *node);
    int (*alloc_inode)();
    void (*free_inode)(int inum);
    int (*grow_inode)(spfs_inode_t *node, int size);
    int (*shrink_inode)(spfs_inode_t *node, int size);
    int (*inode_get_pnum)(spfs_inode_t *node, int fpn);
}spfs_op_t;


typedef struct spfs_inode {
  int refs;  // reference count
  int mode;  // permission & type
  int size;  // bytes
  int block; // single block pointer (if max file size <= 4K)

  time_t atime; // access time
  time_t mtime; // modify time
  time_t ctime; // change time

  spfs_op_t i_op;
}spfs_inode_t;


#endif
