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


set(FUSE_CODES_DIR ${SRC_DIR}/fuse)
file(GLOB_RECURSE FUSE_SRC_CODES ${FUSE_CODES_DIR}/*.c)

set(FS_CODES_DIR ${SRC_DIR}/fs)
file(GLOB_RECURSE  FS_SRC_CODES ${FS_CODES_DIR}/*.c)

set(BLOCK_CODES_DIR ${SRC_DIR}/block)
file(GLOB_RECURSE BLOCK_SRC_CODES ${BLOCK_CODES_DIR}/*.c)

#include_directories(${COMM_CODES_DIR}/include)
include_directories(${LIBFUSE_SRC_DIR}/include)

message(STATUS ${LIBFUSE_SRC_DIR})

set (CODE_SRC ${FUSE_SRC_CODES}
    ${FS_SRC_CODES}
    ${BLOCK_SRC_CODES}
    ${SRC_DIR}/ysfs_config.c)

add_executable(${BIN_NAME} ${CURRENTDIR}/fusefs_main.c ${CODE_SRC})
target_link_libraries(${BIN_NAME} pthread dl)
target_link_libraries(${BIN_NAME} ${LIBFUSE_SRC_DIR}/libfuse3.a)


file(COPY ${LIBFUSE_SRC_DIR}/libfuse3.so.3 DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/) 
file(COPY ${LIBFUSE_SRC_DIR}/libfuse3.so DESTINATION ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/) 

#INSTALL(TARGETS ${BIN_NAME}
#    RUNTIME DESTINATION bin
#    LIBRARY DESTINATION lib
#)

#target_link_libraries(${BIN_NAME} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})

