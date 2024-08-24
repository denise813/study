#ifndef __JNI_SO_H
#define __JNI_SO_H


#ifdef __cplusplus
extern "C" {
#endif


#include "stdint.h"
#include "list.h"


#define JNI_SO_CLUSTER_MAGIC 0x90424
#define JNI_SO_BLOCK_SIZE 4096
#define JNI_SO_GB (1LL << 30)


enum JNI_SO_ERROR_CODE
{
    JNI_SO_ERROR_CODE_CREATE_EXTERNAL = 36012201,
    JNI_SO_ERROR_CODE_ZBS_ADDR,
    JNI_SO_ERROR_CODE_QUERY_OFFSET_OR_LENGHT,
    JNI_SO_ERROR_CODE_END,
};

typedef struct jni_so_volume_extent
{
    uint64_t jsve_index;
    uint64_t jsve_offset;
    uint64_t jsve_length;
    struct list_head node;
}jni_so_volume_extent_t;


typedef struct jni_so_area_context
{
    uint64_t jsac_diff_num;
    struct list_head jsac_extent_list;
}jni_so_area_context_t;


typedef struct jni_so_context
{
    char * jsc_process_name;
    char * jsc_host;
    int jsc_blocksize;
    uint64_t jsc_magic;
}jni_so_context_t;


int jni_so_create_cluster_context(
                const char * process_name,
                const char * host,
                jni_so_context_t ** context,
                char ** err_massge);
int jni_so_delete_cluster_context(
                jni_so_context_t * jni_so_context);
int jni_so_check_cluster_context(
                jni_so_context_t * jni_so_context);
int jni_so_create_alloc_query(
                jni_so_context_t * jni_so_context,
                const char * snap_local_id,
                uint64_t offset,
                uint64_t lenght,
                jni_so_area_context_t ** context);
int jni_so_delete_alloc_query(
                jni_so_area_context_t * context);
int jni_so_create_change_query(
                jni_so_context_t * jni_so_context,
                const char * snap1_local_id,
                const char * snap2_local_id,
                uint64_t offset,
                uint64_t lenght,
                jni_so_area_context_t ** context);
int jni_so_delete_change_query(
                jni_so_area_context_t * context);
int jni_so_volume_read(
                jni_so_context_t * jni_so_context,
                const char * volume_id,
                uint64_t offset,
                uint64_t length,
                char * buffer);
int jni_so_volume_write(
                jni_so_context_t * jni_so_context,
                const char * volume_id,
                uint64_t offset,
                uint64_t length,
                char * buffer);

int jni_so_set_log_level(int level);


#ifdef __cplusplus
}
#endif


#endif  // __YS_ZADP_SO_H
