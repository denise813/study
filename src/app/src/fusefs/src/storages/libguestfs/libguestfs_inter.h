#ifndef LIBGUESTFS_INTER_H
#define LIBGUESTFS_INTER_H


#include <guestfs.h>


#ifdef __cplusplus
extern "C"
{
#endif


typedef struct libguestfs_config
{
    char ** gfs_bdevs[100];
    char * gfs_root;
    int gfs_bdevs_num;
}libguestfs_config_t;


typedef struct libguestfs_mount_info
{
    char * m_init_dev;
    char ** m_part_dev[100];
    int m_partnum;
}libguestfs_mount_info_t;


typedef struct libguestfs
{
    libguestfs_config_t gfs_config;
    storage_op_t * gfs_op;
    guestfs_h * gfs_libgfs;
    libguestfs_mount_info_t gfs_mountinfo;
}libguestfs_t;


typedef struct libguestfs_dir
{
    char * d_path;
    struct dirent * d_itor;
    int d_index;
    struct guestfs_dirent_list * d_ents;
}libguestfs_dir_t;

typedef struct libguestfs_file
{
    char * f_path;
}libguestfs_file_t;


int libguestfs_malloc_fs(void * fuse_storage);
void libguestfs_free_fs(void * fuse_storage);


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
