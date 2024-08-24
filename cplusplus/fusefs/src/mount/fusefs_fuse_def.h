#ifndef YSFS_FUSE_DEF_H
#define YSFS_FUSE_DEF_H


#if 0
#define FUSE_USE_VERSION 26
#ifdef linux
// needed for a few things fuse depends on
#define _XOPEN_SOURCE 700
#endif
#include <fuse/fuse.h>
#include <fuse/fuse_lowlevel.h>
#else
#define _GNU_SOURCE
#define FUSE_USE_VERSION 30
#include "fuse.h"
#include "fuse_lowlevel.h"
#include "fuse_common.h"


#endif

#ifdef __cplusplus
extern "C"
{
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif


#endif
