#ifndef __YS_JNI_SO_EXPORT_H
#define __YS_JNI_SO_EXPORT_H


#include "jni.h"


#ifdef __cplusplus
extern "C" {
#endif


JNIEXPORT jint JNICALL Java_com_demo_Native_createClusterContext(
                JNIEnv * env,
                jobject clazz,
                jstring j_process_name,
                jstring j_host,
                jobject j_cluster_obj);
JNIEXPORT jint JNICALL Java_com_demo_Native_deleteClusterContext(
                JNIEnv * env,
                jobject obj,
                jlong j_zadp_context_addr);
JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_allocQuery(
                JNIEnv * env,
                jobject clazz,
                jlong j_zadp_context_addr,
                jstring j_snap_local_id,
                jlong j_offset,
                jlong j_length,
                jobject j_list_obj,
                jstring j_extent_cls_full_name);
JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_changeQuery(
                JNIEnv * env,
                jobject clazz,
                jlong j_zadp_context_addr,
                jstring j_snap1_local_id,
                jstring j_snap2_local_id,
                jlong j_offset,
                jlong j_length,
                jobject j_list_obj,
                jstring j_extent_cls_full_name);
JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_volumeRead(
                JNIEnv * env,
                jobject clazz,
                jlong j_zadp_context_addr,
                jstring j_volume_id,
                jlong j_offset,
                jlong j_length,
                jobject j_byteBuffer);
JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_volumeWrite(
                JNIEnv * env,
                jobject clazz,
                jlong j_zadp_context_addr,
                jstring j_volume_id,
                jlong j_offset,
                jlong j_length,
                jobject j_byteBuffer);
JNIEXPORT jint JNICALL Java_com_sea_shell_demo_Native_setLoglevel(
                JNIEnv * env,
                jobject clazz,
                jint j_log_level);


#ifdef __cplusplus
}
#endif


#endif  // __YS_ZADP_SO_H
