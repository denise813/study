cmake_minimum_required(VERSION 3.16)
project(fusefs)

set(CURRENTDIR ${CMAKE_CURRENT_SOURCE_DIR})

# project
set(BIN_NAME "fusefs")

set(SRC_DIR ${CURRENTDIR}/src)
#set(COMM_CODES_DIR ${CURRENTDIR}/../comm/)

#add_definitions(-DD_FILE_OFFSET_BITS 64)
set(THRIDPARTY_SRC_DIR ${CURRENTDIR}/../thridparty)
set(LIBFUSE_SRC_DIR ${THRIDPARTY_SRC_DIR}/fuse3)
set(LIBGUESTFS_SRC_DIR ${THRIDPARTY_SRC_DIR}/libguestfs)

set(FUSE_CODES_DIR ${SRC_DIR})
set(FUSE_MOUNT_CODES_DIR ${SRC_DIR}/mount)
set(STORAGE_CODES_DIR ${FUSE_MOUNT_CODES_DIR}/storages)

file(GLOB_RECURSE FUSE_SRC_CODES
	${FUSE_CODES_DIR}/fusefs_config.c
    ${FUSE_MOUNT_CODES_DIR}/fusefs_fuse_fs.c
    ${FUSE_MOUNT_CODES_DIR}/fusefs_fuse_fs_op.c
    ${FUSE_MOUNT_CODES_DIR}/fusefs_storage.c
    ${FUSE_MOUNT_CODES_DIR}/fusefs_storage_op.c)

set(PASSFS_STORAGE_CODES_DIR ${STORAGE_CODES_DIR}/passfs)
file(GLOB_RECURSE  PASSFS_STORAGE_SRC_CODES
    ${PASSFS_STORAGE_CODES_DIR}/passfs_inter.c)

if (HAVE_GUESTFS_BACKEND)
set(GUESTFS_STORAGE_CODES_DIR ${STORAGE_CODES_DIR}/libguestfs)
file(GLOB_RECURSE  GUESTFS_STORAGE_SRC_CODES
    ${GUESTFS_STORAGE_CODES_DIR}/libguestfs_inter.c)
add_definitions(-DSTORAGE_ENABLE_BACKEND_GUESTFS=1)
else(HAVE_GUESTFS_BACKEND)
set(GUESTFS_STORAGE_SRC_CODES "")
endif(HAVE_GUESTFS_BACKEND)
    
include_directories(${LIBGUESTFS_SRC_DIR}/include)
#include_directories(${COMM_CODES_DIR}/include)
include_directories(${LIBFUSE_SRC_DIR}/include)

message(STATUS ${LIBFUSE_SRC_DIR})

set (CODE_SRC
    ${FUSE_SRC_CODES}
    ${PASSFS_STORAGE_SRC_CODES})

add_executable(${BIN_NAME} ${CURRENTDIR}/fusefs_main.c ${CODE_SRC})
target_link_libraries(${BIN_NAME} pthread dl)
target_link_libraries(${BIN_NAME} ${LIBFUSE_SRC_DIR}/libfuse3.a)

if (HAVE_GUESTFS_BACKEND)
target_link_libraries(${BIN_NAME} guestfs)
endif(HAVE_GUESTFS_BACKEND)

file(COPY ${LIBFUSE_SRC_DIR}/libfuse3.so.3 DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/) 
file(COPY ${LIBFUSE_SRC_DIR}/libfuse3.so DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/) 

#INSTALL(TARGETS ${BIN_NAME}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#)

#target_link_libraries(${BIN_NAME} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

