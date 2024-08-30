set(BIN_NAME "struct2json")


set(CURRENTDIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CURRENT_SRC_DIR ${CURRENTDIR}/src)
set(CJSON_SRC_DIR ${THIRD_PARTY_PATH}/cjson)


include_directories(${CJSON_SRC_DIR})
include_directories(${CURRENT_SRC_DIR})


file(GLOB_RECURSE CJSON_SRC_CODES
    ${CJSON_SRC_DIR}/*.cc
    ${CJSON_SRC_DIR}/*.c)
file(GLOB_RECURSE STRUCT2JSON_SRC_CODES
    ${CURRENT_SRC_DIR}/*.cc
    ${CURRENT_SRC_DIR}/*.c)

set(CODE_SRC ${STRUCT2JSON_SRC_CODES} ${CJSON_SRC_CODES})

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -save-temps=obj")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -save-temps=obj")

if (HAVE_CPPCHECK_CONTEXT)
    set(CEHCK_SRCS ${CODE_SRC})
    execute_process( COMMAND ${CPPCHECK_FLAG} ${CEHCK_SRCS}
                RESULT_VARIABLE CMAKE_CXX_CPPCHECK_RETURN )
    if(CMAKE_CXX_CPPCHECK_RETURN)
        message(FATAL_ERROR "CppCheck failed: ${CMAKE_CXX_CPPCHECK_RETURN}" )
    endif(CMAKE_CXX_CPPCHECK_RETURN)
endif(HAVE_CPPCHECK_CONTEXT)

#message(STATUS "CJSON_SRC_DIR : ${CJSON_SRC_DIR}" )

add_executable(${BIN_NAME} ${CURRENTDIR}/struct2json_main.cc ${CODE_SRC})
target_link_libraries(${BIN_NAME} pthread dl)

install(TARGETS ${BIN_NAME})

