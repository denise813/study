#include <stdlib.h>
#include <string.h>
#include <errno.h>


#include "jni_log.h"
#include "jni_so.h"



static int dump_jni_so_context(jni_so_context_t * context)
{
    JNI_INFO("jni_so_context-> { \n"
                    "jni_so_process_name:%s, \n"
                    "jni_so_host:%s, \n"
                    "jni_so_magic:%lx, \n"
                    "jni_so_blocksize:%d \n"
                    "}",
                    context->jsc_process_name,
                    context->jsc_host,
                    context->jsc_magic,
                    context->jsc_blocksize);
    return 0;
}

static int dump_jni_so_area_context(jni_so_area_context_t * context)
{
    JNI_TRACE("jni_so_area_context-> { \n"
                    "jni_soa_diff_num:%lu, \n"
                    "jni_soa_extent_list:%p, \n"
                    "}",
                    context->jsac_diff_num,
                    &context->jsac_extent_list);
    return 0;
}

#if 0
static int dump_jni_so_volume_extent(jni_so_volume_extent_t * context)
{
    JNI_DEBUG("jni_so_volume_extent-> { \n"
                    "jni_sove_index:%lu, \n"
                    "jni_sove_offset:%lu, \n"
                    "jni_sove_length:%lu, \n"
                    "}",
                    context->jscve_index,
                    context->jscve_offset,
                    context->jscve_length);
    return 0;
}
#endif

static int dump_jni_so_volume_io(
                const char *volume_id,
                uint64_t offset,
                uint64_t length,
                int rc)
{
    JNI_DEBUG("jni_so_volume_io-> { \n"
                    "volume_id:%s, \n"
                    "offset:%lu, \n"
                    "length:%lu, \n"
                    "rc:%d \n"
                    "}",
                    volume_id,
                    offset,
                    length,
                    rc);
    return 0;
}

static int __jni_so_clear_extent_list(struct list_head * list)
{
    jni_so_volume_extent_t * next = NULL;
    jni_so_volume_extent_t * pos = NULL;

    list_for_each_entry_safe(pos, next, list, node) {
        list_del_init(&pos->node);
        free(pos);
    }

    return 0;
}

int jni_so_create_cluster_context(
                const char * process_name,
                const char * host,
                jni_so_context_t ** context,
                char **err_massage)
{
    int rc = 0;
 //   char * err = NULL;
    jni_so_context_t * jni_so_context = (jni_so_context_t*)malloc(sizeof(jni_so_context_t));
    if (!jni_so_context) {
        rc = -ENOMEM;
        JNI_ERROR("malloc jni_so_context_t failed, rc=(%d)", rc);
        goto l_out;
    }
    jni_so_context->jsc_magic = JNI_SO_CLUSTER_MAGIC;
    jni_so_context->jsc_process_name = strdup(process_name);
    if (!jni_so_context->jsc_process_name) {
        rc = -ENOMEM;
        JNI_ERROR("malloc jni_so_process_name failed, rc=(%d)", rc);
        goto l_free_jni_so;
    }
    jni_so_context->jsc_host = strdup(host);
     if (!jni_so_context->jsc_host) {
        rc = -ENOMEM;
        JNI_ERROR("malloc jni_so_host failed, rc=(%d)", rc);
        goto l_free_process_name;
    }
    jni_so_context->jsc_blocksize = JNI_SO_BLOCK_SIZE;
    dump_jni_so_context(jni_so_context);
    *context = jni_so_context;
    rc = 0;

l_out:
    return rc;

//l_free_host:
//    free(jni_so_context->jsc_host);
l_free_process_name:
    free(jni_so_context->jsc_process_name);
l_free_jni_so:
    free(jni_so_context);
    goto l_out;
}

int jni_so_delete_cluster_context(
                jni_so_context_t * jni_so_context)
{
    if (!jni_so_context) {
        return 0;
    }

    if (jni_so_context->jsc_process_name) {
        free(jni_so_context->jsc_process_name);
        jni_so_context->jsc_process_name = NULL;
    }

    if (jni_so_context->jsc_host) {
        free(jni_so_context->jsc_host);
        jni_so_context->jsc_host = NULL;
    }

    return 0;
}

int jni_so_check_cluster_context(
                jni_so_context_t * jni_so_context)
{
    int rc = 0;

    if (!jni_so_context) {
        rc = -JNI_SO_ERROR_CODE_ZBS_ADDR;
        JNI_ERROR("jni_so_check_cluster_context jni_so_context failed, rc=(%d)", rc);
        goto l_out;
    }

    if (jni_so_context->jsc_magic != JNI_SO_CLUSTER_MAGIC) {
        rc = -JNI_SO_ERROR_CODE_ZBS_ADDR;
         JNI_ERROR("jni_so_check_cluster_context jni_so_context failed, "
                         "jni_so_magic(%lx), "
                        " rc=(%d)",
                         jni_so_context->jsc_magic,
                         rc);
        goto l_out;
    }
    rc = 0;

l_out:
    return rc;
}

int jni_so_create_alloc_query(
                jni_so_context_t * jni_so_context,
                const char * snap_local_id,
                uint64_t offset,
                uint64_t length,
                jni_so_area_context_t ** context)
{
    //uint64_t index = 0;
    int rc = 0;
    //jni_so_volume_extent_t * extent = NULL;
    jni_so_area_context_t * jni_so_area_context = NULL;

    if ((length % JNI_SO_GB) || (offset % JNI_SO_GB)) {
        rc = -JNI_SO_ERROR_CODE_QUERY_OFFSET_OR_LENGHT;
        goto l_out;
    }
    jni_so_area_context = (jni_so_area_context_t*)malloc(sizeof(jni_so_area_context_t));
    if (!jni_so_area_context) {
        rc = -ENOMEM;
        goto l_out;
    }
    jni_so_area_context->jsac_diff_num = 0;

    INIT_LIST_HEAD(&jni_so_area_context->jsac_extent_list);
    // to be continue
    dump_jni_so_area_context(jni_so_area_context);
    *context = jni_so_area_context;
    rc = 0;

l_out:
    return rc;

//l_free_list:
//    __jni_so_clear_extent_list(&jni_so_area_context->jsac_extent_list);
//    jni_so_area_context->jsac_diff_num = 0;

//l_free_context:
//    free(jni_so_area_context);
    goto l_out;
}

int jni_so_delete_alloc_query(
                jni_so_area_context_t * context)
{
    if (context == NULL) {
        return 0;
    }

    if (!list_empty(&context->jsac_extent_list)) {
        __jni_so_clear_extent_list(&context->jsac_extent_list);
    }
    return 0;
}

int jni_so_create_change_query(
                jni_so_context_t * jni_so_context,
                const char * snap1_local_id,
                const char * snap2_local_id,
                uint64_t offset,
                uint64_t length,
                jni_so_area_context_t ** context)
{
    //uint64_t index = 0;
    int rc = 0;
    //jni_so_volume_extent_t * extent = NULL;
    jni_so_area_context_t * jni_so_area_context = NULL;

    if ((length % JNI_SO_GB) || (offset % JNI_SO_GB)) {
        rc = -JNI_SO_ERROR_CODE_QUERY_OFFSET_OR_LENGHT;
        goto l_out;
    }
    jni_so_area_context = (jni_so_area_context_t*)malloc(sizeof(jni_so_area_context_t));
    if (!jni_so_area_context) {
        rc = -ENOMEM;
        goto l_out;
    }
    jni_so_area_context->jsac_diff_num = 0;

    INIT_LIST_HEAD(&jni_so_area_context->jsac_extent_list);
    // to be continue
    dump_jni_so_area_context(jni_so_area_context);
    *context = jni_so_area_context;
    rc = 0;

l_out:
    return rc;

//l_free_list:
//    __jni_so_clear_extent_list(&jni_so_area_context->jsac_extent_list);

//l_free_context:
//    free(jni_so_area_context);
    goto l_out;
}

int jni_so_delete_change_query(
                jni_so_area_context_t * context)
{
    if (context == NULL) {
        return 0;
    }

    if (!list_empty(&context->jsac_extent_list)) {
        __jni_so_clear_extent_list(&context->jsac_extent_list);
    }
    return 0;
}

int jni_so_volume_read(
                jni_so_context_t * jni_so_context,
                const char * volume_id,
                uint64_t offset,
                uint64_t length,
                char * buffer)
{
    int rc = 0;
    if (length % JNI_SO_BLOCK_SIZE != 0 || offset % JNI_SO_BLOCK_SIZE) {
        rc = -JNI_SO_ERROR_CODE_QUERY_OFFSET_OR_LENGHT;
        goto l_out;
    }

    dump_jni_so_volume_io(volume_id,offset, length, rc);

l_out:
    return rc;
}

int jni_so_volume_write(
                jni_so_context_t * jni_so_context,
                const char * volume_id,
                uint64_t offset,
                uint64_t length,
                char * buffer)
{
    int rc = 0;
    if (length % JNI_SO_BLOCK_SIZE != 0 || offset % JNI_SO_BLOCK_SIZE) {
        rc = -JNI_SO_ERROR_CODE_QUERY_OFFSET_OR_LENGHT;
        goto l_out;
    }

    dump_jni_so_volume_io(volume_id,offset, length, rc);

l_out:
    return rc;
}

int jni_so_set_log_level(int level)
{
    return 0;
}

