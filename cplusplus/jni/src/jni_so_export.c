#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>

#include "jni.h"
#include "jni_log.h"
#include "jni_so.h"
#include "jni_so_export.h"


#define SO_JNI_LOAD_METHOD_NAME "jni_load"
#define SO_CLUSTER_JNI_LOAD_METHOD_PAR "(LL;)V"
//#define SO_VOLUME_AREA_JNI_LOAD_METHOD_PAR "(Ljava/util/List;)V"
#define SO_JNI_INIT_METHOD_NAME "<init>"
#define SO_VOLUME_ETENT_JNI_INIT_METHOD_PAR "(JJ)V"
#define SO_JNI_LIST_ADD_METHOD_NAME "add"
#define SO_JNI_LIST_ADD_METHOD_PAR "(Ljava/lang/Object;)Z"


#define SO_CLUSTER_JNI_PAR_ZADP_ADDR_NAME "jni_so_addr"
#define SO_CLUSTER_JNI_PAR_ZADP_ADDR_TYPE "J"
#define SO_VOLUME_AREA_JNI_PAR_EXTENT_NAME "jni_so_extent_list"
#define SO_VOLUME_AREA_JNI_PAR_EXTENT_TYPE "Ljava/util/ArrayList;"
#define SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME "position"
#define SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE "I"


enum JNI_SO_EXPORT_ERROR_CODE
{
    JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR = JNI_SO_ERROR_CODE_END + 1,
    JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_CLUSTER,
    JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_AREA,
    JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_EXTENT,
    JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_LIST,
    JNI_SO_EXPORT_ERROR_CODE_BUILD_CLUSTER,
    JNI_SO_EXPORT_ERROR_CODE_BUILD_AREA,
    JNI_SO_EXPORT_ERROR_CODE_ALLOC_EXTENT,
    JNI_SO_EXPORT_ERROR_CODE_ALLOC_LIST,
    JNI_SO_EXPORT_ERROR_CODE_ADDR,
    JNI_SO_EXPORT_ERROR_CODE_BUFFER,
    JNI_SO_EXPORT_ERROR_CODE_END,
};

static int java_ZADPClass_jni_so_native_check_extent_cls(JNIEnv * env,
                const char * exetent_cls_path)
{
    int rc = 0;
    jclass j_extent_cls;
    jmethodID j_extent_init_mid;

    j_extent_cls = (*env)->FindClass(env, exetent_cls_path);
    if (j_extent_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_EXTENT;
        JNI_ERROR("FindClass failed exetent_cls_path(%s), rc(%d)",
                        exetent_cls_path,
                        rc);
        goto l_out;
    }
    j_extent_init_mid = (*env)->GetMethodID(env, j_extent_cls,
                    SO_JNI_INIT_METHOD_NAME,
                    SO_VOLUME_ETENT_JNI_INIT_METHOD_PAR);
    if (j_extent_init_mid == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_EXTENT;
        JNI_ERROR("(%s) GetMethodID failed "
                        "MethodID(%s),"
                        "Methodargs (%s) "
                        " rc(%d)",
                         exetent_cls_path,
                        SO_JNI_INIT_METHOD_NAME,
                        SO_VOLUME_ETENT_JNI_INIT_METHOD_PAR,
                        rc);
        goto l_free_cls;
    }
    (*env)->DeleteLocalRef(env, j_extent_cls);
    rc = 0;

l_out:
    return rc;
l_free_cls:
    (*env)->DeleteLocalRef(env, j_extent_cls);
    goto l_out;

}

static int java_ZADPClass_jni_so_native_check_cluster_cls(
                JNIEnv * env,
                jobject j_cluster_obj)
{
    int rc = 0;
    jclass j_cluster_cls;
    jfieldID jni_zasp_addr_field_id;

    j_cluster_cls = (*env)->GetObjectClass(env, j_cluster_obj);
    if (j_cluster_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_CLUSTER;
        JNI_ERROR("cluster class GetObjectClass failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
/* --comment by louting, 2024/8/16--
 * ptr of so context
 */
    jni_zasp_addr_field_id = (*env)->GetFieldID(env,
                    j_cluster_cls,
                    SO_CLUSTER_JNI_PAR_ZADP_ADDR_NAME,
                    SO_CLUSTER_JNI_PAR_ZADP_ADDR_TYPE);
    if (jni_zasp_addr_field_id == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_BUILD_CLUSTER;
        JNI_ERROR("cluster class GetFieldID failed,"
                        "GetFieldID(%s) "
                        "FieldIDtype(%s), "
                        "rc(%d)",
                        SO_CLUSTER_JNI_PAR_ZADP_ADDR_NAME,
                        SO_CLUSTER_JNI_PAR_ZADP_ADDR_TYPE,
                        rc);
        goto l_free_cls;
    }
    (*env)->DeleteLocalRef(env, j_cluster_cls);

l_out:
    return rc;
l_free_cls:
    (*env)->DeleteLocalRef(env, j_cluster_cls);
    goto l_out;
}

static int java_ZADPClass_jni_so_native_check_area_cls(
                JNIEnv * env,
                jobject j_list_obj,
                const char * exetent_cls_path)
{
    int rc = 0;
    jclass j_list_cls;
    jmethodID j_list_add_mid;

    j_list_cls = (*env)->GetObjectClass(env, j_list_obj);
    if (j_list_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_AREA;
        JNI_ERROR("list class GetObjectClass failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    j_list_add_mid = (*env)->GetMethodID(env, j_list_cls,
                    SO_JNI_LIST_ADD_METHOD_NAME,
                    SO_JNI_LIST_ADD_METHOD_PAR);
    if (j_list_add_mid == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_LIST;
        JNI_ERROR("list class GetMethodID failed,"
                         "Method %s "
                         "MethodArgs %s "
                        " rc(%d)",
                        SO_JNI_LIST_ADD_METHOD_NAME,
                        SO_JNI_LIST_ADD_METHOD_PAR,
                        rc);
        goto l_free_list_cls;
    }
    rc = java_ZADPClass_jni_so_native_check_extent_cls(env, exetent_cls_path);
    if (rc < 0) {
        goto l_free_list_cls;
    }
    (*env)->DeleteLocalRef(env, j_list_cls);

l_out:
    return rc;
l_free_list_cls:
    (*env)->DeleteLocalRef(env, j_list_cls);
    goto l_out;
}

static int java_ZADPClass_jni_so_native_check_buffer_cls(
                JNIEnv * env,
                jobject j_buff_obj)
{
    int rc = 0;
    jclass j_buff_cls;
    jfieldID jni_buffer_position_field_id;

    j_buff_cls = (*env)->GetObjectClass(env, j_buff_obj);
    if (j_buff_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_AREA;
        JNI_ERROR("list class GetObjectClass failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    jni_buffer_position_field_id = (*env)->GetFieldID(
                    env,
                    j_buff_cls,
                    SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME,
                    SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE);
    if (jni_buffer_position_field_id == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_LIST;
        JNI_ERROR("buffer class GetFieldID failed,"
                         "field %s "
                         "filedType %s "
                        " rc(%d)",
                        SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME,
                        SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE,
                        rc);
        goto l_free_buffer_cls;
    }
    (*env)->DeleteLocalRef(env, j_buff_cls);

l_out:
    return rc;
l_free_buffer_cls:
    (*env)->DeleteLocalRef(env, j_buff_cls);
    goto l_out;
}


static jobject java_ZADPClass_jni_so_native_create_extent_object(JNIEnv * env,
                const char * exetent_cls_path,
                uint64_t offset,
                uint64_t length)
{
    jclass j_extent_cls;
    jmethodID j_extent_init_mid;
    jobject j_extent_obj;
    jlong j_offset;
    jlong j_ength;

    j_extent_cls = (*env)->FindClass(env, exetent_cls_path);
    if (j_extent_cls == NULL) {
        JNI_ERROR("FindClass failed exetent_cls_path(%s)",
                    exetent_cls_path);
        goto l_out;
    }
/* --comment by louting, 2024/8/16--
 * ptr of context, recode of interface
 */
    j_extent_init_mid =
                    (*env)->GetMethodID(env, j_extent_cls,
                                    SO_JNI_INIT_METHOD_NAME,
                                    SO_VOLUME_ETENT_JNI_INIT_METHOD_PAR);
    if (j_extent_init_mid == NULL) {
        JNI_ERROR("GetMethodID failed exetent_cls_path(%s) "
                        "MethodName(%s) "
                        "MethodArgs (%s) ",
                        exetent_cls_path,
                        SO_JNI_INIT_METHOD_NAME,
                        SO_VOLUME_ETENT_JNI_INIT_METHOD_PAR);
        goto l_free_cls;
    }
    j_offset = (jlong)(offset);
    j_ength = (jlong)(length);
    j_extent_obj = (*env)->NewObject(env, j_extent_cls, j_extent_init_mid, j_offset, j_ength);
    (*env)->DeleteLocalRef(env, j_extent_cls);

l_out:
    return j_extent_obj;
l_free_cls:
    (*env)->DeleteLocalRef(env, j_extent_cls);
    goto l_out;
}

static int java_ZADPClass_jni_so_native_build_cluster_object(
                JNIEnv * env,
                jobject j_cluster_obj,
                jni_so_context_t * context_addr)
{
    int rc = 0;
    jclass j_cluster_cls;
    jfieldID jni_zasp_addr_field_id;
    jlong j_context_ptr;

    j_cluster_cls =  (*env)->GetObjectClass(env, j_cluster_obj);
    if (j_cluster_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_BUILD_CLUSTER;
         JNI_ERROR("claster FindClass failed "
                        "rc(%d). ",
                        rc);
        goto l_out;
    }
    jni_zasp_addr_field_id = (*env)->GetFieldID(env,
                    j_cluster_cls,
                    SO_CLUSTER_JNI_PAR_ZADP_ADDR_NAME,
                    SO_CLUSTER_JNI_PAR_ZADP_ADDR_TYPE);
    if (jni_zasp_addr_field_id == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_BUILD_CLUSTER;
        JNI_ERROR("cluster class GetFieldID failed,"
                        "GetFieldID(%s) "
                        "FieldIDtype(%s), "
                        "rc(%d)",
                        SO_CLUSTER_JNI_PAR_ZADP_ADDR_NAME,
                        SO_CLUSTER_JNI_PAR_ZADP_ADDR_TYPE,
                        rc);
        goto l_free_cls;
    }
    j_context_ptr =(jlong)(context_addr);
    (*env)->SetLongField(
                    env,
                    j_cluster_obj,
                    jni_zasp_addr_field_id,
                    j_context_ptr);
    (*env)->DeleteLocalRef(env, j_cluster_cls);
    rc = 0;

l_out:
    return rc;
l_free_cls:
    (*env)->DeleteLocalRef(env, j_cluster_cls);
    goto l_out;
}

static int java_ZADPClass_jni_so_native_build_area_object(
                JNIEnv * env,
                jobject j_list_obj,
                const char * exetent_cls_path,
                struct list_head * list)
{
    int rc = 0;
    jclass j_list_cls;
    jmethodID j_list_add_mid;
    jobject j_extent_obj;
    jni_so_volume_extent_t * itor = NULL;

    j_list_cls = (*env)->GetObjectClass(env, j_list_obj);
    if (j_list_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_AREA;
        JNI_ERROR("list class GetObjectClass failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    j_list_add_mid = (*env)->GetMethodID(env, j_list_cls,
                    SO_JNI_LIST_ADD_METHOD_NAME,
                    SO_JNI_LIST_ADD_METHOD_PAR);
    if (j_list_add_mid == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_LIST;
        JNI_ERROR("list class GetMethodID failed,"
                        "Method %s "
                        "MethodArgs %s "
                        " rc(%d)",
                        SO_JNI_LIST_ADD_METHOD_NAME,
                        SO_JNI_LIST_ADD_METHOD_PAR,
                        rc);
        goto l_free_list_cls;
    }

    list_for_each_entry(itor, list, node) {
        j_extent_obj = java_ZADPClass_jni_so_native_create_extent_object(
                        env,
                        exetent_cls_path,
                        itor->jsve_offset,
                        itor->jsve_length);
        if (!j_extent_obj) {
            rc = -JNI_SO_EXPORT_ERROR_CODE_ALLOC_EXTENT;
            JNI_ERROR("java_ZADPClass_jni_so_native_create_extent_object failed,"
                            "exetent_cls_path(%s) "
                            " rc(%d)",
                            exetent_cls_path,
                            rc);
            goto l_free_list_cls;
        }
        (*env)->CallBooleanMethod(env, j_list_obj, j_list_add_mid, j_extent_obj);
        (*env)->DeleteLocalRef(env, j_extent_obj);
    }

l_out:
    return rc;
l_free_list_cls:
    (*env)->DeleteLocalRef(env, j_list_cls);
    goto l_out;
}

JNIEXPORT jint JNICALL Java_com_demo_Native_createClusterContext(
                JNIEnv * env,
                jobject clazz,
                jstring j_process_name,
                jstring j_host,
                jobject j_cluster_obj)
{
    int rc = 0;
    jni_so_context_t * context = NULL;
    const char *process_name = NULL;
    const char *host = NULL;
    char * so_err_message = NULL;

    process_name = (*env)->GetStringUTFChars(env, j_process_name, 0);
    if (!process_name) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
        JNI_ERROR("createClusterContext get process_name failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    host = (*env)->GetStringUTFChars(env, j_host, 0);
    if (!host) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
        JNI_ERROR("createClusterContext get host failed,"
                        " rc(%d)",
                        rc);
        goto l_free_process_name;
    }

    JNI_INFO("createClusterContext Java_com_demo_Native_createClusterContext args,"
                        " process_name (%s) "
                        " host(%s) ",
                        process_name,
                        host);
    rc = java_ZADPClass_jni_so_native_check_cluster_cls(env, j_cluster_obj);
    if (rc < 0) {
        JNI_ERROR("createClusterContext java_ZADPClass_jni_so_native_check_cluster_cls failed,"
                        " rc(%d)",
                        rc);
        goto l_free_host;
    }
    rc = jni_so_create_cluster_context(process_name, host, &context, &so_err_message);
    if (rc < 0) {
         JNI_ERROR("createClusterContext so_create_cluster_context failed,"
                        "process_name(%s) "
                        " rc(%d)",
                        process_name,
                        rc);
        goto l_free_host;
    }
    rc = java_ZADPClass_jni_so_native_build_cluster_object(env, j_cluster_obj, context);
    if (rc < 0) {
        goto l_free_cluster_context;
    }
    JNI_INFO("createClusterContext java_ZADPClass_jni_so_native_build_cluster_object out,"
                        " process_name (%s) "
                        " host(%s) "
                        "context(%p) ",
                        process_name,
                        host,
                        context);
    (*env)->ReleaseStringUTFChars(env, j_process_name, process_name);
    (*env)->ReleaseStringUTFChars(env, j_host, host);
    rc = 0;

l_out:
    return (jint)(rc);

l_free_cluster_context:
     jni_so_delete_cluster_context(context);
     if (so_err_message) {
        free(so_err_message);
        so_err_message = NULL;
     }
l_free_host:
    (*env)->ReleaseStringUTFChars(env, j_host, host);
l_free_process_name:
     (*env)->ReleaseStringUTFChars(env, j_process_name, process_name);
      if (rc > 0) { rc = -rc; }
    goto l_out;
}

JNIEXPORT jint JNICALL Java_com_demo_Native_deleteClusterContext(
                JNIEnv * env,
                jobject obj,
                jlong j_jni_so_context_addr)
{
    int rc = 0;
    jni_so_context_t * jni_so_extent = (jni_so_context_t*)(j_jni_so_context_addr);
    rc = jni_so_check_cluster_context(jni_so_extent);
    if (rc < 0){
        rc = -JNI_SO_EXPORT_ERROR_CODE_ADDR;
         JNI_INFO("deleteClusterContext Java_com_demo_Native_deleteClusterContext arg,"
                        "context(%p) ",
                        jni_so_extent);
        goto l_out;
    }
    (void)jni_so_delete_cluster_context(jni_so_extent);
    rc = 0;

l_out:
     if (rc > 0) { rc = -rc; }
    return (jint)(rc);
}

JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_allocQuery(
                JNIEnv * env,
                jobject clazz,
                jlong j_jni_so_context_addr,
                jstring j_snap_local_id,
                jlong j_offset,
                jlong j_length,
                jobject j_list_obj,
                jstring j_extent_cls_full_name)
{
    int rc = 0;
    jni_so_context_t * so_context = NULL;
    jni_so_area_context_t * context = NULL;
    const char *extent_cls_path = NULL;
    const char *snap_local_id = NULL;
    uint64_t offset = 0;
    uint64_t length = 0;

    so_context = (jni_so_context_t*)(j_jni_so_context_addr);
    rc = jni_so_check_cluster_context(so_context);
    if (rc < 0){
        JNI_ERROR("jni_so_check_cluster_context failed,"
                        "so_context (%p) "
                        " rc(%d)",
                        so_context,
                        rc);
        goto l_out;
    }
    offset = (uint64_t)j_offset;
    length = (uint64_t)j_length;
    snap_local_id = (*env)->GetStringUTFChars(env, j_snap_local_id, 0);
    if (!snap_local_id) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
        JNI_ERROR("check snap_local_id failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    extent_cls_path = (*env)->GetStringUTFChars(env, j_extent_cls_full_name, 0);
    if (!extent_cls_path) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
        JNI_ERROR("check extent_cls_path failed,"
                        "snap_local_id "
                        " rc(%d)",
                        rc);
        goto l_free_snap_id;
    }
     rc = java_ZADPClass_jni_so_native_check_area_cls(env, j_list_obj, extent_cls_path);
    if (rc < 0) {
        JNI_ERROR("check java_ZADPClass_jni_so_native_check_area_cls failed,"
                        "extent_cls_path (%s) "
                        " rc(%d)",
                        extent_cls_path,
                        rc);
        goto l_free_extent_cls_name;
    }

    JNI_INFO("Java_com_sea_shell_demo_Native_allocQuery args,"
                    "so_context (%p),"
                    "offset(%lu) "
                    "length(%lu) "
                    "snap_local_id(%s) "
                    "extent_cls_path (%s) ",
                    so_context,
                    offset,
                    length,
                    snap_local_id,
                    extent_cls_path);

    rc = jni_so_create_alloc_query(
                    so_context,
                    snap_local_id,
                    offset,
                    length,
                    &context);
    if (rc < 0) {
         JNI_ERROR("so_create_alloc_query failed,"
                        " rc(%d)",
                        rc);
        goto l_free_extent_cls_name;
    }
    rc = java_ZADPClass_jni_so_native_build_area_object(
                    env,
                    j_list_obj,
                    extent_cls_path,
                    &context->jsac_extent_list);
    if (rc < 0) {
        JNI_ERROR("java_ZADPClass_jni_so_native_build_area_object failed,"
                        " rc(%d)",
                        rc);
        goto l_free_alloc_query;
    }
    (*env)->ReleaseStringUTFChars(env, j_snap_local_id, snap_local_id);
    (*env)->ReleaseStringUTFChars(env, j_extent_cls_full_name, extent_cls_path);
    jni_so_delete_alloc_query(context);
    rc = 0;

l_out:
    return (jint)(rc);

l_free_alloc_query:
    jni_so_delete_alloc_query(context);
l_free_extent_cls_name:
    (*env)->ReleaseStringUTFChars(env, j_extent_cls_full_name, extent_cls_path);
l_free_snap_id:
    (*env)->ReleaseStringUTFChars(env, j_snap_local_id, snap_local_id);
     if (rc > 0) { rc = -rc; }
    goto l_out;
}

JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_changeQuery(
                JNIEnv * env,
                jobject clazz,
                jlong j_jni_so_context_addr,
                jstring j_snap1_local_id,
                jstring j_snap2_local_id,
                jlong j_offset,
                jlong j_length,
                jobject j_list_obj,
                jstring j_extent_cls_full_name)
{
    int rc = 0;
    jni_so_context_t * so_context =NULL;
    jni_so_area_context_t * context = NULL;
    const char *extent_cls_path = NULL;
    const char * snap1_local_id = NULL;
    const char * snap2_local_id = NULL;
    uint64_t offset = 0;
    uint64_t length = 0;

    so_context = (jni_so_context_t*)(j_jni_so_context_addr);
    rc = jni_so_check_cluster_context(so_context);
    if (rc < 0){
        rc = -JNI_SO_EXPORT_ERROR_CODE_ADDR;
        JNI_ERROR("jni_so_check_cluster_context failed,"
                        "so_context (%p) "
                        " rc(%d)",
                        so_context,
                        rc);
        goto l_out;
    }
    offset = (uint64_t)j_offset;
    length = (uint64_t)j_length;
    snap1_local_id = (*env)->GetStringUTFChars(env, j_snap1_local_id, 0);
    if (!snap1_local_id) {
         rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
          JNI_ERROR("check snap1_local_id failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    snap2_local_id = (*env)->GetStringUTFChars(env, j_snap2_local_id, 0);
    if (!snap2_local_id) {
         rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
         JNI_ERROR("check snap2_local_id failed,"
                        " rc(%d)",
                        rc);
        goto l_free_snap1_id;
    }
    extent_cls_path = (*env)->GetStringUTFChars(env, j_extent_cls_full_name, 0);
    if (!extent_cls_path) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
        JNI_ERROR("check extent_cls_path failed,"
                        " rc(%d)",
                        rc);
        goto l_free_snap2_id;
    }
    JNI_INFO("Java_com_sea_shell_demo_Native_changeQuery args,"
                    "so_context (%p),"
                    "offset(%lu) "
                    "length(%lu) "
                    "snap1_local_id(%s) "
                    "snap2_local_id(%s) "
                    "extent_cls_path (%s) ",
                    so_context,
                    offset,
                    length,
                    snap1_local_id,
                    snap2_local_id,
                    extent_cls_path);
    rc = java_ZADPClass_jni_so_native_check_area_cls(env, j_list_obj, extent_cls_path);
    if (rc < 0) {
        JNI_ERROR("check java_ZADPClass_jni_so_native_check_area_cls failed,"
                        " rc(%d)",
                        rc);
        goto l_free_snap2_id;
    }
    rc = jni_so_create_change_query(
                    so_context,
                    snap1_local_id,
                    snap2_local_id,
                    offset,
                    length,
                    &context);
    if (rc < 0) {
        JNI_ERROR("check so_create_change_query failed,"
                        " rc(%d)",
                        rc);
        goto l_free_extent_cls_path_id;
    }

    rc = java_ZADPClass_jni_so_native_build_area_object(
                    env,
                    j_list_obj,
                    extent_cls_path,
                    &context->jsac_extent_list);
    if (rc < 0) {
        JNI_ERROR("check java_ZADPClass_jni_so_native_build_area_object failed,"
                        " rc(%d)",
                        rc);
        goto l_free_change_query;
    }
    (*env)->ReleaseStringUTFChars(env, j_snap1_local_id, snap1_local_id);
    (*env)->ReleaseStringUTFChars(env, j_snap2_local_id, snap2_local_id);
    (*env)->ReleaseStringUTFChars(env, j_extent_cls_full_name, extent_cls_path);
    jni_so_delete_change_query(context);
    rc = 0;

l_out:
    return (jint)(rc);

l_free_change_query:
    jni_so_delete_change_query(context);
l_free_extent_cls_path_id:
    (*env)->ReleaseStringUTFChars(env, j_extent_cls_full_name, extent_cls_path);
l_free_snap2_id:
    (*env)->ReleaseStringUTFChars(env, j_snap2_local_id, snap2_local_id);
l_free_snap1_id:
    (*env)->ReleaseStringUTFChars(env, j_snap1_local_id, snap1_local_id);
    if (rc > 0) { rc = -rc; }
    goto l_out;
}

JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_volumeRead(
                JNIEnv * env,
                jobject clazz,
                jlong j_jni_so_context_addr,
                jstring j_volume_id,
                jlong j_offset,
                jlong j_length,
                jobject j_byteBuffer)
{
    int rc = 0;
    uint64_t offset = 0;
    uint64_t length = 0;
    char* buffer = NULL;
    jni_so_context_t * so_context = NULL;
    const char * volume_id = NULL;
    jclass j_buff_cls;
    jfieldID jni_buffer_position_field_id;

    so_context = (jni_so_context_t*)(j_jni_so_context_addr);
    rc = jni_so_check_cluster_context(so_context);
    if (rc < 0){
        rc = -JNI_SO_EXPORT_ERROR_CODE_ADDR;
        JNI_ERROR("jni_so_check_cluster_context failed,"
                        "so_context (%p) "
                        " rc(%d)",
                        so_context,
                        rc);
        goto l_out;
    }
    rc = java_ZADPClass_jni_so_native_check_buffer_cls(env, j_byteBuffer);
    if (rc < 0) {
        JNI_ERROR("java_ZADPClass_jni_so_native_check_buffer_cls failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }

    j_buff_cls = (*env)->GetObjectClass(env, j_byteBuffer);
    if (j_buff_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_AREA;
        JNI_ERROR("list class GetObjectClass failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    jni_buffer_position_field_id = (*env)->GetFieldID(
                    env,
                    j_buff_cls,
                    SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME,
                    SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE);
    if (jni_buffer_position_field_id == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_LIST;
        JNI_ERROR("buffer class GetFieldID failed,"
                         "field %s "
                         "filedType %s "
                        " rc(%d)",
                        SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME,
                        SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE,
                        rc);
        goto l_free_buffer_cls;
    }
    buffer = (char*)((*env)->GetDirectBufferAddress(env, j_byteBuffer));
    if (buffer == NULL) {
        rc  =-JNI_SO_EXPORT_ERROR_CODE_BUFFER;
        JNI_ERROR("jni_so_check_cluster_context failed,"
                        "so_context (%p) "
                        " rc(%d)",
                        so_context,
                        rc);
        goto l_free_buffer_cls;
    }
    volume_id = (*env)->GetStringUTFChars(env, j_volume_id, 0);
    if (!volume_id) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_ADDR;
        JNI_ERROR("check volume_id failed,"
                        " rc(%d)",
                        rc);
        goto l_free_buffer_cls;
    }
    offset = (uint64_t)j_offset;
    length = (uint64_t)j_length;
    JNI_DEBUG("Java_com_sea_shell_demo_Native_volumeRead args,"
                    "so_context (%p),"
                    "offset(%lu) "
                    "length(%lu) "
                    "volume_id(%s) ",
                    so_context,
                    offset,
                    length,
                    volume_id);
    rc = jni_so_volume_read(so_context, volume_id, offset, length, buffer);
    if (rc < 0) {
        JNI_ERROR("so_volume_read failed,"
                        "volume_id(%s) "
                        " rc(%d)",
                        volume_id,
                        rc);
        goto l_free_vol_id;
    }
    rc = (int)(length);
    (*env)->SetIntField(
                    env,
                    j_byteBuffer,
                    jni_buffer_position_field_id,
                    rc);
    JNI_DEBUG("Java_com_sea_shell_demo_Native_volumeRead args,"
                    "so_context (%p),"
                    "offset(%lu) "
                    "length(%lu) "
                    "volume_id(%s) "
                    "readsize(%d)",
                    so_context,
                    offset,
                    length,
                    volume_id,
                    rc);
    (*env)->ReleaseStringUTFChars(env, j_volume_id, volume_id);
    (*env)->DeleteLocalRef(env, j_buff_cls);
    rc = 0;

l_out:
    return (jint)(rc);

l_free_vol_id:
     (*env)->ReleaseStringUTFChars(env, j_volume_id, volume_id);
l_free_buffer_cls:
    (*env)->DeleteLocalRef(env, j_buff_cls);
     if (rc > 0) { rc = -rc; }
    goto l_out;
}

JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_volumeWrite(
                JNIEnv * env,
                jobject clazz,
                jlong j_jni_so_context_addr,
                jstring j_volume_id,
                jlong j_offset,
                jlong j_length,
                jobject j_byteBuffer)
{
    int rc = 0;
    uint64_t offset = 0;
    uint64_t length = 0;
    char* buffer = NULL;
    jni_so_context_t * so_context = NULL;
    jclass j_buff_cls;
    jfieldID jni_buffer_position_field_id;

    so_context = (jni_so_context_t*)(j_jni_so_context_addr);
    rc = jni_so_check_cluster_context(so_context);
    if (rc < 0){
        rc = -JNI_SO_EXPORT_ERROR_CODE_ADDR;
        JNI_ERROR("jni_so_check_cluster_context failed,"
                        "so_context (%p) "
                        " rc(%d)",
                        so_context,
                        rc);
        goto l_out;
    }
    rc = java_ZADPClass_jni_so_native_check_buffer_cls(env, j_byteBuffer);
    if (rc < 0) {
        JNI_ERROR("java_ZADPClass_jni_so_native_check_buffer_cls failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }

    j_buff_cls = (*env)->GetObjectClass(env, j_byteBuffer);
    if (j_buff_cls == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_AREA;
        JNI_ERROR("list class GetObjectClass failed,"
                        " rc(%d)",
                        rc);
        goto l_out;
    }
    jni_buffer_position_field_id = (*env)->GetFieldID(
                    env,
                    j_buff_cls,
                    SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME,
                    SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE);
    if (jni_buffer_position_field_id == NULL) {
        rc = -JNI_SO_EXPORT_ERROR_CODE_INVAL_CLS_LIST;
        JNI_ERROR("buffer class GetFieldID failed,"
                         "field %s "
                         "filedType %s "
                        " rc(%d)",
                        SO_VOLUME_BUFFER_JNI_PAR_POSITION_NAME,
                        SO_VOLUME_BUFFER_JNI_PAR_POSITION_TYPE,
                        rc);
        goto l_free_buffer_cls;
    }

    buffer = (char*)((*env)->GetDirectBufferAddress(env, j_byteBuffer));
    if (buffer == NULL) {
        rc  =-JNI_SO_EXPORT_ERROR_CODE_BUFFER;
        JNI_ERROR("check volume_id failed,"
                        " rc(%d)",
                        rc);
        goto l_free_buffer_cls;
    }
    const char * volume_id = (*env)->GetStringUTFChars(env, j_volume_id, 0);
    if (!volume_id) {
        rc  =-JNI_SO_EXPORT_ERROR_CODE_INVAL_PAR;
         JNI_ERROR("check volume_id failed,"
                        " rc(%d)",
                        rc);
        goto l_free_buffer_cls;
    }
    offset = (uint64_t)j_offset;
    length = (uint64_t)j_length;
    JNI_DEBUG("Java_com_sea_shell_demo_Native_volumeWrite args,"
                    "so_context (%p),"
                    "offset(%lu) "
                    "length(%lu) "
                    "volume_id(%s) ",
                    so_context,
                    offset,
                    length,
                    volume_id);
    rc = jni_so_volume_write(so_context, volume_id, offset, length, buffer);
    if (rc < 0) {
       JNI_ERROR("so_volume_read failed,"
                        "volume_id(%s) "
                        " rc(%d)",
                        volume_id,
                        rc);
        goto l_free_vol_id;
    }
    rc = (int)(length);
    (*env)->SetIntField(
                    env,
                    j_byteBuffer,
                    jni_buffer_position_field_id,
                    rc);
    JNI_DEBUG("Java_com_sea_shell_demo_Native_volumeWrite args,"
                    "so_context (%p),"
                    "offset(%lu) "
                    "length(%lu) "
                    "volume_id(%s) "
                    "writesize (%d)",
                    so_context,
                    offset,
                    length,
                    volume_id,
                    rc);
    (*env)->ReleaseStringUTFChars(env, j_volume_id, volume_id);
    (*env)->DeleteLocalRef(env, j_buff_cls);
    rc = 0;

l_out:
    return(jint)(rc);

l_free_vol_id:
     (*env)->ReleaseStringUTFChars(env, j_volume_id, volume_id);
 l_free_buffer_cls:
    (*env)->DeleteLocalRef(env, j_buff_cls);
     if (rc > 0) { rc = -rc; }
    goto l_out;
}

JNIEXPORT jint JNICALL Java_com_sea_shell_demo_jni_SmartxNative_setLoglevel(
                JNIEnv * env,
                jobject clazz,
                jint j_log_level)
{
    return (jint)(0);
}


