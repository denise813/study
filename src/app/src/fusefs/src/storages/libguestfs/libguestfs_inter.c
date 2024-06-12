#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "../../fusefs_storage_op.h"
#include "../../fusefs_storage.h"
#include "../../fusefs_log.h"
#include "libguestfs_inter.h"
#include <guestfs.h>


/* --comment by louting, 2024/5/13--
 * 
 */

#define LIBGUESTFS_PATH_MAX 1024
#define LIBGUESTFS_IO_LIMIT  2 * 1024 * 1024

static int libguestfs_print_string(char ** string_buff)
{
    int index = 0;
    for(index=0;; index++) {
        if(!string_buff[index]) { break; }
        FUSEFS_DEBUG("%s\n", string_buff[index]);
    }
    return 0;
}

static int libguestfs_get_os_info(guestfs_h * g, char * os_dev, char ** os_info)
{
    int rc = 0;
    char *info = NULL;

    FUSEFS_TRACE("guestfs_inspect_get_osinfo enter");
    info = guestfs_inspect_get_osinfo(g, os_dev);
    if (info == NULL) {
        rc = -1;
        FUSEFS_ERROR("guestfs_inspect_get_osinfo failed");
        goto l_out;
    }
    *os_info = strdup(info);
    rc = 0;
    FUSEFS_DEBUG("guestfs_inspect_get_osinfo os(%s), info(%s)", os_dev, *os_info);

l_out:
    FUSEFS_TRACE("guestfs_inspect_get_osinfo exit");
    return rc;
}

static int libguestfs_init(void * private)
{
    int rc = 0;
    int index = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);
    FUSEFS_TRACE("libguestfs_init enter ");
    
    for (index = 0; index < fs->gfs_config.gfs_bdevs_num; index++) {
        rc = guestfs_add_drive(fs->gfs_libgfs, (char *)fs->gfs_config.gfs_bdevs[index]);
        if (rc< 0) {
            FUSEFS_ERROR("guestfs_add_drive failed\n");
            rc = -1;
            goto l_out;
        }
    }

    rc = guestfs_launch(fs->gfs_libgfs);
    if (rc < 0) {
        FUSEFS_ERROR("guestfs_launch failed");
        rc = -1;
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_init ok");

l_out:
    FUSEFS_TRACE("libguestfs_init exit");
    return rc;
}

static int libguestfs_exit(void * private)
{
    libguestfs_t * fs = (libguestfs_t *)(private);
    FUSEFS_TRACE("libguestfs_exit enter");

    FUSEFS_TRACE("libguestfs_exit exit");
    return 0;
}


static int libguestfs_mount(void * private)
{
    int rc = 0;
    int index = 0;
    int partid = 0;
    char ** guest_os_devs = NULL;
    char ** guest_fs_devs = NULL;
    char ** guest_mountpoints = NULL;
    char * itor_os_info = NULL;
    char * init_dev = NULL;
    char itor_part_dir[21] = {0};
    char * dest_part = NULL;
    size_t size = 0;

    libguestfs_t * fs = (libguestfs_t *)(private);
    FUSEFS_TRACE("libguestfs_mount enter ");

    guest_os_devs = guestfs_inspect_os(fs->gfs_libgfs);
    if (guest_os_devs == NULL) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_inspect_get_osinfo failed ");
        rc = -rc;
        goto l_out;
    }

    FUSEFS_TRACE("os");
    libguestfs_print_string(guest_os_devs);
    for(index = 0;;index++) {
        if (!guest_os_devs[index]) {break;}
        libguestfs_get_os_info(fs->gfs_libgfs, guest_os_devs[index], &itor_os_info);
        if (strcmp("cirros2011.8", itor_os_info) == 0) {
            fs->gfs_mountinfo.m_init_dev = strdup(guest_os_devs[index]);
        }
        free(itor_os_info);
        itor_os_info = NULL;
    }

    if (fs->gfs_mountinfo.m_init_dev == NULL) {
        rc = -1;
        goto l_out;
    }

    rc = guestfs_mount(fs->gfs_libgfs, fs->gfs_mountinfo.m_init_dev, "/");
    if (rc == -1) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_mount failed ");
        rc = -rc;
        goto l_out;
    }

    rc = guestfs_mkdir(fs->gfs_libgfs, "/sysroot");
    if (rc == -1) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_mkdir failed ");
        rc = -rc;
        goto l_umount;
    }
    guest_fs_devs = guestfs_list_filesystems(fs->gfs_libgfs);
    if (guest_fs_devs == NULL) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_list_filesystems failed");
        rc = -rc;
        goto l_rmrootdir;
    }
    libguestfs_print_string(guest_fs_devs);
    for(index = 0;;index++) {
      if (!guest_fs_devs[index]) {break;}
      if (index % 2 == 1) {continue;}
      if (strcmp(guest_fs_devs[index], fs->gfs_mountinfo.m_init_dev) == 0) { index++; continue;}
  #if 0
      guest_mountpoints = guestfs_inspect_get_mountpoints(fs->gfs_libgfs, guest_fs_devs[index]);
      if (guest_mountpoints != NULL) {
          fprintf(stderr, "guestfs_inspect_get_mountpoints %s\n", guest_fs_devs[index]);
          libguestfs_print_string(guest_mountpoints);
      }
  #endif
      size = snprintf(itor_part_dir, 20, "/sysroot/part_%d", partid);
      rc = guestfs_mkdir(fs->gfs_libgfs, itor_part_dir);
      if (rc == -1) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_mkdir failed\n");
        rc = -rc;
        goto l_rmpartdir;
      }
      dest_part = (char*)fs->gfs_mountinfo.m_part_dev[partid];
      dest_part = strdup(itor_part_dir);
      partid = partid + 1;
    }
    fs->gfs_mountinfo.m_partnum = partid;
    rc = 0;

l_out:
    FUSEFS_TRACE("libguestfs_mount exit");
    return rc;

l_rmpartdir:
    for(index = 0; index < partid; index++) {
        snprintf(itor_part_dir, 20, "/sysroot/part_%d", partid);
        guestfs_rmdir(fs->gfs_libgfs, itor_part_dir);
    }
l_rmrootdir:
    guestfs_rmdir(fs->gfs_libgfs, "/sysroot");
l_umount:
    guestfs_umount(fs->gfs_libgfs, "/");
    goto l_out;
}

static int libguestfs_umount(void * private)
{
    int rc = 0;
    int index = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);
    char itor_part_dir[21] = {0};

    FUSEFS_TRACE("libguestfs_umount enter");

    for(index = 0; index < fs->gfs_mountinfo.m_partnum; index++) {
        snprintf(itor_part_dir, 20, "/sysroot/part_%d", index);
        guestfs_rmdir(fs->gfs_libgfs, itor_part_dir);
    }
    rc = guestfs_umount(fs->gfs_libgfs, "/");
    if (rc < 0) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_umount failed\n");
        rc = -rc;
        goto l_out;
    }

l_out:
    FUSEFS_TRACE("libguestfs_umount exit");
    return 0;
}

static int libguestfs_trans_path(void * private, const char * path, char * real_path)
{
    libguestfs_t * fs = (libguestfs_t*)private;
    libguestfs_config_t * config = &fs->gfs_config;

    snprintf(real_path, LIBGUESTFS_PATH_MAX, "%s", config->gfs_root, path);
    return 0;
}

static int libguestfs_infs(void * private, const char * path)
{
    FUSEFS_TRACE("libguestfs_infs enter %s", path);
    FUSEFS_TRACE("libguestfs_infs exit %s", path);
    return 0;
}

static int libguestfs_stat(void * private, const char *path, struct statvfs *s)
{
    int rc= 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    struct guestfs_statvfs * guest_s = NULL;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_stat enter %s-%s", path, real_path);


    guest_s = guestfs_statvfs(fs->gfs_libgfs, real_path);
     if (!guest_s) {
        rc = guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("statvfs failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    s->f_bsize = guest_s->bsize;
    s->f_frsize = guest_s->frsize;
    s->f_blocks = guest_s->blocks;
    s->f_bfree = guest_s->bfree;
    s->f_bavail = guest_s->bavail;
    s->f_files = guest_s->files;
    s->f_ffree = guest_s->ffree;
    s->f_favail = guest_s->favail;
    s->f_fsid = guest_s->fsid;
    s->f_flag = guest_s->flag;
    s->f_namemax = guest_s->namemax;

    guestfs_free_statvfs(guest_s);
    rc = 0;
    FUSEFS_TRACE("libguestfs_stat ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_stat exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_getattr(void * private, const char *path, struct stat *s)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);
    struct guestfs_stat * guest_s = NULL;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_getattr enter %s-%s", path, real_path);

    guest_s = guestfs_lstat(fs->gfs_libgfs, path);
    if (!guest_s) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_lstat failed %s-%s", path, real_path);
        goto l_out;
    }

    s->st_dev = guest_s->dev;
    s->st_ino = guest_s->ino;
    s->st_mode = guest_s->mode;
    s->st_nlink = guest_s->nlink;
    s->st_uid = guest_s->uid;
    s->st_gid = guest_s->gid;
    s->st_rdev = guest_s->rdev;
    s->st_size = guest_s->size;
    s->st_blksize = guest_s->blksize;
    s->st_blocks = guest_s->blocks;
    s->st_atime = guest_s->atime;
    s->st_mtime = guest_s->mtime;
    s->st_ctime = guest_s->ctime;
    guestfs_free_stat(guest_s);

    rc = 0;
    FUSEFS_TRACE("libguestfs_getattr ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_getattr exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_access(void * private, const char *path, int mask)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_access enter %s-%s", path, real_path);

    struct stat statbuf;

    rc = libguestfs_getattr(private, path, &statbuf);
    if (rc < 0) {
         FUSEFS_TRACE("fg_getattr ok %s-%s", path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_access ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_access exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_mkdir(void * private, const char *path, mode_t mode)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_mkdir enter %s-%s", path, real_path);

    rc = guestfs_mkdir_mode(fs->gfs_libgfs, real_path, mode);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("access failed, errno(%d) %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_mkdir ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_mkdir exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_opendir(void * private, const char *path, void **dp)
{
    int rc = 0;
    libguestfs_dir_t * libguestfs_dp = NULL;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_opendir enter %s-%s", path, real_path);

    libguestfs_dp->d_path = strdup(path);
    libguestfs_dp->d_index = 0;
    libguestfs_dp->d_itor = malloc(sizeof(struct dirent));
    if (!libguestfs_dp->d_itor) {
      rc = -ENOMEM;
      goto l_free_path;
    }
    *dp = (void*)libguestfs_dp;
    rc = 0;
    FUSEFS_TRACE("libguestfs_opendir ok %s-%s %p", path, real_path, *dp);

l_out:
    FUSEFS_TRACE("libguestfs_opendir exit %s-%s %p", path, real_path);
    return rc;

l_free_path:
    free(libguestfs_dp->d_path);
    libguestfs_dp->d_path = NULL;
    goto l_out;
}

static int libguestfs_closedir(void * private, void *dp)
{
    libguestfs_dir_t * libguestfs_dp = (libguestfs_dir_t*)dp;

    FUSEFS_TRACE("libguestfs_closedir enter %p", dp);
    free(libguestfs_dp->d_itor);
    free(libguestfs_dp->d_path);
    FUSEFS_TRACE("libguestfs_closedir ok %p", dp);

    FUSEFS_TRACE("libguestfs_closedir exit %p", dp);
    return 0;
}

static int libguestfs_readdir(void * private, void *dp,
                    struct dirent **de)
{
    int rc = 0;
    libguestfs_dir_t *libguestfs_dp = NULL;
    libguestfs_t * fs = (libguestfs_t *)(private);

    FUSEFS_TRACE("libguestfs_readdir enter %p", dp);
    libguestfs_dp = (libguestfs_dir_t*)dp;

    if (!libguestfs_dp->d_ents) {
        libguestfs_dp->d_ents = guestfs_readdir(fs->gfs_libgfs, libguestfs_dp->d_path);
    }
    if (libguestfs_dp->d_ents == NULL) {
      rc = guestfs_last_errno(fs->gfs_libgfs);
      rc = -rc;
      goto l_out;
    }

    if (libguestfs_dp->d_index == libguestfs_dp->d_ents->len) {
        rc = 0;
        goto l_out;
    }

    libguestfs_dp->d_itor->d_ino = libguestfs_dp->d_ents->val[libguestfs_dp->d_index].ino;
    memcpy(libguestfs_dp->d_itor->d_name, libguestfs_dp->d_ents->val[libguestfs_dp->d_index].name, 512);

    switch (libguestfs_dp->d_ents->val[libguestfs_dp->d_index].ftyp) {
        case 'b': libguestfs_dp->d_itor->d_type = DT_BLK; break;
        case 'c': libguestfs_dp->d_itor->d_type = DT_CHR; break;
        case 'd': libguestfs_dp->d_itor->d_type = DT_DIR; break;
        case 'f': libguestfs_dp->d_itor->d_type = DT_FIFO; break;
        case 'l': libguestfs_dp->d_itor->d_type = DT_LNK; break;
        case 'r': libguestfs_dp->d_itor->d_type = DT_REG; break;
        case 's': libguestfs_dp->d_itor->d_type = DT_SOCK; break;
        case 'u':
        case '?':
        default:  libguestfs_dp->d_itor->d_type = DT_UNKNOWN;
    }

    *de = libguestfs_dp->d_itor;
    rc = 0;
    FUSEFS_TRACE("libguestfs_readdir ok %p", dp);

l_out:
    FUSEFS_TRACE("libguestfs_readdir exit %p", dp);
    return rc;
}

static int libguestfs_rename(void * private, const char *from, const char *to)
{
    int rc = 0;
    char libguestfs_path_from[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path_from = libguestfs_path_from;
    char libguestfs_path_to[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path_to = libguestfs_path_to;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, to, libguestfs_path_from);
    libguestfs_trans_path(private, to, libguestfs_path_to);
    FUSEFS_TRACE("libguestfs_rename enter %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

    rc = guestfs_mv(fs->gfs_libgfs, libguestfs_path_from, libguestfs_path_to);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("rename failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        libguestfs_path_from,
                        libguestfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_rename ok %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

l_out:
    FUSEFS_TRACE("libguestfs_rename exit %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);
    return rc;
}

static int libguestfs_unlink(void * private, const char *path)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_unlink enter. %s-%s", path, real_path);
    rc = guestfs_rm(fs->gfs_libgfs, real_path);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_rm failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_unlink ok. %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("libguestfs_unlink exit. %s-%s", path, real_path);
    return rc;
}

static int libguestfs_open(void * private, const char *path, int flags, void **file)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_file_t * guestfs_file = NULL;

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_open enter %s-%s", path, real_path);

    guestfs_file = (libguestfs_file_t *)malloc(sizeof(libguestfs_file_t));
    if (!guestfs_file) {
        rc = -ENOMEM;
        FUSEFS_ERROR("malloc failed %d %s-%s", rc, path, real_path);
         goto l_out;
    }

    guestfs_file->f_path = strdup(path);
    rc = 0;
    *file = (void*)guestfs_file;
    FUSEFS_TRACE("libguestfs_open ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_open exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_close(void * private, void *file)
{
    libguestfs_file_t * guestfsfile = NULL;

    FUSEFS_TRACE("libguestfs_close enter");

    guestfsfile = (libguestfs_file_t*)(file);
    free(guestfsfile->f_path);
    free(guestfsfile);
    FUSEFS_TRACE("libguestfs_close exit");

l_out:
    FUSEFS_TRACE("libguestfs_close exit");
    return 0;
}

static ssize_t libguestfs_read(void * private, void * file, char *buff, size_t buff_size,
                    off_t off)
{
    int rc = 0;
    libguestfs_file_t * guestfile = NULL;
    ssize_t size = 0;
    size_t r_buff_size = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    FUSEFS_TRACE("libguestfs_read enter");

    if (buff_size > LIBGUESTFS_IO_LIMIT) {
        r_buff_size = LIBGUESTFS_IO_LIMIT;
    } else {
        r_buff_size = buff_size;
    }

    guestfile = (libguestfs_file_t*)(file);
    
    buff = guestfs_pread(fs->gfs_libgfs, guestfile->f_path, r_buff_size, off, &size);
    if (size < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        size  = rc;
        FUSEFS_ERROR("read failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_read ok");

l_out:
    FUSEFS_TRACE("libguestfs_read exit");
    return size;
}

static ssize_t libguestfs_write(void * private,
                void * file,
                const char *buff,
                size_t buff_size,
                off_t off)
{
    int rc = 0;
    libguestfs_file_t * guestfile = NULL;
    ssize_t size = 0;
    size_t w_buff_size = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    FUSEFS_TRACE("libguestfs_write enter");

    guestfile = (libguestfs_file_t*)(file);
    if (buff_size > LIBGUESTFS_IO_LIMIT) {
        w_buff_size = LIBGUESTFS_IO_LIMIT;
    } else {
        w_buff_size = buff_size;
    }

    size = guestfs_pwrite(fs->gfs_libgfs, guestfile->f_path, buff, w_buff_size, off);
    if (size < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        size  = rc;
        FUSEFS_ERROR("write failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_write ok");

l_out:
    FUSEFS_TRACE("libguestfs_write exit");
    return size;
}

static int libguestfs_fsync(void * private,
                void * file,
                int isdatasync)
{
    int rc = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    FUSEFS_TRACE("libguestfs_fsync enter");
    rc = guestfs_sync(fs->gfs_libgfs);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_sync failed %d", rc);
        goto l_out;
    }
    rc = 0;

l_out:
    FUSEFS_TRACE("libguestfs_fsync exit");
    return rc;
}

static int libguestfs_ftruncate(void * private,void * file, off_t size)
{
    int rc = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);
    libguestfs_file_t * guestfile = (libguestfs_file_t*)file;

    FUSEFS_TRACE("libguestfs_ftruncate enter");

    rc = guestfs_truncate_size(fs->gfs_libgfs, guestfile->f_path, size);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("ftruncate failed %d", rc);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_ftruncate ok");

l_out:
    FUSEFS_TRACE("libguestfs_ftruncate exit");
    return rc;
}

static int libguestfs_truncate(void * private,const char *path, off_t size)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_truncate enter %s-%s", path, real_path);

    rc = guestfs_truncate_size(fs->gfs_libgfs, real_path, size);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("truncate failed %d %s-%s", rc, path, real_path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_truncate ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("libguestfs_truncate exit %s-%s", path, real_path);
    return 0;
}

static int libguestfs_symlink(void * private,const char *from, const char *to)
{
    int rc = 0;
    char libguestfs_path_from[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path_from = libguestfs_path_from;
     char libguestfs_path_to[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path_to = libguestfs_path_to;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, from, libguestfs_path_from);
    libguestfs_trans_path(private, to, libguestfs_path_to);
    FUSEFS_TRACE("libguestfs_symlink enter %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

    rc = guestfs_ln_s(fs->gfs_libgfs, libguestfs_path_from, libguestfs_path_to);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("symlink failed %d %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        libguestfs_path_from,
                        libguestfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_symlink ok %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

l_out:
    FUSEFS_TRACE("libguestfs_symlink exit %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);
    return rc;
}
    // unsupported functions
static int libguestfs_link(void * private,const char *from, const char *to)
{
    int rc = 0;
    char libguestfs_path_from[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path_from = libguestfs_path_from;
    char libguestfs_path_to[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path_to = libguestfs_path_to;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, from, real_path_from);
    libguestfs_trans_path(private, to, real_path_to);
    FUSEFS_TRACE("libguestfs_link enter %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

    rc = guestfs_ln(fs->gfs_libgfs, real_path_from, real_path_to);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_TRACE("link failed errno(%d) %s-%s, %s-%s",
                        rc,
                        from,
                        to,
                        libguestfs_path_from,
                        libguestfs_path_to);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_link ok %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);

l_out:
    FUSEFS_TRACE("libguestfs_link exit %s-%s, %s-%s",
                    from,
                    to,
                    libguestfs_path_from,
                    libguestfs_path_to);
    return rc;
}

static int libguestfs_mknod(void * private,const char *path, mode_t mode, dev_t dev)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_mknod enter %s-%s", path, real_path);

     rc = guestfs_mknod(fs->gfs_libgfs, mode, major(dev), minor(dev), path);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        goto l_out;
    }
    rc = 0;
    FUSEFS_TRACE("libguestfs_mknod ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_mknod exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_chmod(void * private,const char *path, mode_t mode)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_chmod enter %s-%s", path, real_path);
    rc = guestfs_chmod(fs->gfs_libgfs, mode, real_path);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("guestfs_chmod failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_chmod ok %s-%s", path, real_path);

l_out:
    FUSEFS_TRACE("libguestfs_chmod exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_chown(void * private,const char *path, uid_t uid, gid_t gid)
{
    int rc = 0;
    char libguestfs_path[LIBGUESTFS_PATH_MAX] = {0};
    char * real_path = libguestfs_path;
    libguestfs_t * fs = (libguestfs_t *)(private);

    libguestfs_trans_path(private, path, real_path);
    FUSEFS_TRACE("libguestfs_chown enter %s-%s", path, real_path);

    rc = guestfs_lchown(fs->gfs_libgfs, uid, gid, real_path);
     if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        FUSEFS_ERROR("libguestfs_chown failed %d %s", rc, path);
        goto l_out;
    }

    rc = 0;
    FUSEFS_TRACE("libguestfs_chown ok %s-%s", path, real_path);

l_out:
     FUSEFS_TRACE("libguestfs_chown exit %s-%s", path, real_path);
    return rc;
}

static int libguestfs_readlink(void * private,const char *path, char *buf, size_t size)
{
    int rc = 0;
    char * guest_buf = NULL;
    size_t len = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    guest_buf = guestfs_readlink (fs->gfs_libgfs, path);
    if (!guest_buf) {
      rc = -guestfs_last_errno(fs->gfs_libgfs);
      goto l_out;
    }
  
    /* Note this is different from the real readlink(2) syscall.  FUSE wants
     * the string to be always nul-terminated, even if truncated.
     */
    len = strlen(guest_buf);
    if (len > size - 1) {
        len = size - 1;
    }
    memcpy (buf, guest_buf, len);
    buf[len] = '\0';
    free(guest_buf);
    rc = 0;

l_out:
    return rc;
}

static int libguestfs_rmdir(void * private,const char *path)
{
    int rc = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    rc = guestfs_rmdir(fs->gfs_libgfs, path);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int libguestfs_setxattr(void * private,
                const char *path,
                const char *name, const char *value,
                size_t size,
                int flags)
{
    int rc = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    rc = guestfs_lsetxattr(fs->gfs_libgfs, name, value, size, path);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

static int libguestfs_getxattr(void * private,
                const char *path,
                const char *name,
                char *value,
                size_t size)
{
    int rc = 0;
    int index = 0;
    int r = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);
    struct guestfs_xattr_list * guest_xattrs = NULL;

    guest_xattrs = guestfs_lgetxattrs(fs->gfs_libgfs, path);
    if (!guest_xattrs) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        goto l_out;
    }

    for (index = 0; index < guest_xattrs->len; index++) {
        if (strcmp(guest_xattrs->val[index].attrname, name) == 0) { break; }
    }

    if (index == guest_xattrs->len) {
        rc = -ENODATA;
        goto l_free_xattr;
    }
  
    /* The getxattr man page is unclear, but if value == NULL then we
     * return the space required (the caller then makes a second syscall
     * after allocating the required amount of space).  If value != NULL
     * then it's not clear what we should do, but it appears we should
     * copy as much as possible and return -ERANGE if there's not enough
     * space in the buffer.
     */
    size_t sz = guest_xattrs->val[index].attrval_len;
    if (value == NULL) {
        rc = sz;
        goto l_free_xattr;
    }

    if (sz <= size) { rc = sz; }
    else { rc = -ERANGE; sz = size; }

    memcpy(value, guest_xattrs->val[index].attrval, sz);
    guestfs_free_xattr_list(guest_xattrs);
    rc = 0;

l_out:
    return rc;

l_free_xattr:
    guestfs_free_xattr_list(guest_xattrs);
    goto l_out;
}


static int libguest_listxattr(void * private,
                const char *path,
                char * list,
                size_t size)
{
    int rc = 0;
    int index = 0;
    size_t space = 0;
    size_t len;
    libguestfs_t * fs = (libguestfs_t *)(private);
    struct guestfs_xattr_list * guest_xattrs = NULL;

    guest_xattrs = guestfs_lgetxattrs(fs->gfs_libgfs, path);
    if (!guest_xattrs) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        goto l_out;
    }

    /* Calculate how much space is required to hold the result. */
    for (index = 0; index < guest_xattrs->len; index) {
        len = strlen(guest_xattrs->val[index].attrname) + 1;
        space += len;
    }

    /* The listxattr man page is unclear, but if list == NULL then we
     * return the space required (the caller then makes a second syscall
     * after allocating the required amount of space).  If list != NULL
     * then it's not clear what we should do, but it appears we should
     * copy as much as possible and return -ERANGE if there's not enough
     * space in the buffer.
     */
    if (!list) {
        rc = space;
        goto l_free_xattr;
    }

    for (index = 0; index < guest_xattrs->len; ++index) {
        len = strlen (guest_xattrs->val[index].attrname) + 1;
        if (size >= len) {
            memcpy(list, guest_xattrs->val[index].attrname, len);
            size -= len;
            list += len;
            rc += len;
        } else {
            rc = -ERANGE;
            break;
        }
    }
    rc = 0;
    guestfs_free_xattr_list(guest_xattrs);

l_out:
    return rc;

l_free_xattr:
    guestfs_free_xattr_list(guest_xattrs);
    goto l_out;
}


static int libguest_removexattr(void * private,
                const char *path,
                const char *name)
{
    int rc = 0;
    libguestfs_t * fs = (libguestfs_t *)(private);

    rc = guestfs_lremovexattr(fs->gfs_libgfs, name, path);
    if (rc < 0) {
        rc = -guestfs_last_errno(fs->gfs_libgfs);
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}


static storage_op_t g_libguestfs_op =
{
    .mount = libguestfs_mount,
    .umount = libguestfs_umount,
    .stat = libguestfs_stat,
    .getattr = libguestfs_getattr,
    .access = libguestfs_access,
    .mkdir = libguestfs_mkdir,
    .opendir = libguestfs_opendir,
    .closedir = libguestfs_closedir,
    .readdir = libguestfs_readdir,
    .rename = libguestfs_rename,
    .unlink = libguestfs_unlink,
    .open = libguestfs_open,
    .close = libguestfs_close,
    .read = libguestfs_read,
    .write = libguestfs_write,
    .fsync = libguestfs_fsync,
    .ftruncate = libguestfs_ftruncate,
    .truncate = libguestfs_truncate,
    .link = libguestfs_link,
    .mknod = libguestfs_mknod,
    .chmod = libguestfs_chmod,
    .chown= libguestfs_chown,
    .infs = libguestfs_infs,
};


int libguestfs_malloc_fs(void * fuse_storage)
{
    int index = 0;
    int rc = 0;
    libguestfs_t * entry = NULL;
    fusefs_storage_t * storage = NULL;
    char * dest_dev = NULL;

    FUSEFS_TRACE("malloc_libguestfs enter");
    storage = (fusefs_storage_t *)fuse_storage;
    entry = malloc(sizeof(libguestfs_t));
    if (!entry) {
        rc = -ENOMEM;
        FUSEFS_TRACE("libguestfs_malloc_fs faild %d", rc);
        goto l_out;
    }
    //entry->libguestfs_block;
    entry->gfs_libgfs = guestfs_create();
    if (!entry->gfs_libgfs) {
        rc = -1;
        goto l_out;
    }

    entry->gfs_op = &g_libguestfs_op;
    for (index = 0; index < storage->s_config.sc_devs_num; index++) {
        dest_dev = (char*)entry->gfs_config.gfs_bdevs[index];
        dest_dev = strdup((char *)storage->s_config.sc_devs[index]);
    }
    entry->gfs_config.gfs_bdevs_num = storage->s_config.sc_devs_num;
    entry->gfs_config.gfs_bdevs[entry->gfs_config.gfs_bdevs_num] = NULL;
    entry->gfs_config.gfs_root = strdup("/");

    storage->s_agent.as_stroage = (void*)entry;
    storage->s_agent.as_stroage_op = &g_libguestfs_op;

    FUSEFS_TRACE("malloc_libguestfs exit");

l_out:
    return rc;
}

void libguestfs_free_fs(void * fuse_storage)
{
    int index = 0;
    libguestfs_t * entry = NULL;
    fusefs_storage_t * storage = NULL;

    FUSEFS_TRACE("free_libguestfs enter");
    storage = (fusefs_storage_t *)fuse_storage;
    entry = (libguestfs_t*)storage->s_agent.as_stroage;

    free(entry->gfs_config.gfs_root);
    for (index = 0; index < entry->gfs_config.gfs_bdevs_num; index++) {
        free(entry->gfs_config.gfs_bdevs[index]);
    }
    
    guestfs_close(entry->gfs_libgfs);
    free(storage->s_agent.as_stroage);
    storage->s_agent.as_stroage = NULL;
    storage->s_agent.as_stroage_op = NULL;

    FUSEFS_TRACE("free_libguestfs exit");
}

