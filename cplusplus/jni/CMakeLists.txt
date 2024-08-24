set(JNI_SO_LIB_NAME "jni_so")
set(JNI_SO_EXPORT_LIB_NAME "jni_export_so")

set(CURRENTDIR ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${CURRENTDIR}/src)

set(LOG_SRC_DIR ${COMM_PATH}/logging)
set(KERNEL_SRC_DIR ${COMM_PATH}/kernel)
set(JNI_SRC_DIR ${THIRD_PARTY_PATH}/jni)

link_directories(${YUANSHUO}/bin) 
include_directories(${KERNEL_SRC_DIR})
include_directories(${LOG_SRC_DIR})
include_directories(${JNI_SRC_DIR}/include)

file(GLOB_RECURSE LOG_SRC_CODES
    ${LOG_SRC_DIR}/*.cc
    ${LOG_SRC_DIR}/*.c)

#link_directories(${ZADP_SRC_DIR}/lib64)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-rpath=${JNI_SO_ORIGIN_RPATH}")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath=${JNI_SO_ORIGIN_RPATH}")

#message(status "LOG_SRC_DIR=${LOG_SRC_DIR}")
set(CODE_SRC ${LOG_SRC_CODES})

add_library(${JNI_SO_LIB_NAME} SHARED ${CURRENTDIR}/src/jni_so.c ${CODE_SRC})
target_link_libraries(${JNI_SO_LIB_NAME} pthread dl)


add_library(${JNI_SO_EXPORT_LIB_NAME} SHARED ${CURRENTDIR}/src/jni_so_export.c ${CODE_SRC})
target_link_libraries(${JNI_SO_EXPORT_LIB_NAME} dl)
target_link_libraries(${JNI_SO_EXPORT_LIB_NAME} ${JNI_SO_LIB_NAME})


install(TARGETS ${JNI_SO_EXPORT_LIB_NAME} LIBRARY)
install(TARGETS ${JNI_SO_LIB_NAME} LIBRARY)
