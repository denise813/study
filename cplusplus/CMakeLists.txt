cmake_minimum_required(VERSION 3.16)
SET(DCMAKE_EXPORT_COMPILE_COMMANDS ON)
project(study)


set(CMAKE_CXX_STANDARD 14) # 设置C++标准为C++14
set(CMAKE_CXX_STANDARD_REQUIRED ON) # 必须使用C++14标准
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

#===================== set path =====================
set(TOP_SRC_DIR ${PROJECT_SOURCE_DIR})

set(CPLUSPLUS_DIR_PATH "${TOP_SRC_DIR}")
set(THIRD_PARTY_PATH "${CPLUSPLUS_DIR_PATH}/thirdparty")
#set(TOOLS_PATH "${TOP_SRC_DIR}/tools")
set(COMM_PATH "${TOP_SRC_DIR}/comm")
#set(ETC_PATH "${TOP_SRC_DIR}/etc")
#set(TST_PATH "${TOP_SRC_DIR}/test")

#===================== set arch =====================
#EXECUTE_PROCESS( COMMAND uname -m COMMAND tr -d '\n' OUTPUT_VARIABLE HARDWARE)
#set(ARCHITECTURE $ENV{OPERATIONSPROTEM}_${HARDWARE})
#message(STATUS "---${ARCHITECTURE}")

#平台检查
#if (CMAKE_SPROTEM_NAME MATCHES "Linux")
#    set (SPROTEM_NAME_LINUX ON)
#endif (CMAKE_SPROTEM_NAME)

#===================== FUNC =====================
#--------------GET_GIT_INFO
function(GET_GIT_INFO
    args_gitname
    FUNC_GET_GIT_INFO_GIT_COMMIT_ID
    FUNC_GET_GIT_INFO_GIT_COMMIT_DATE)
    execute_process(COMMAND git log -1 --format="%H"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE FUNC_GIT_COMMIT_ID)
    execute_process(COMMAND git log -1 --format="%ct"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE FUNC_GIT_COMMIT_DATE)
    string (REGEX REPLACE "[\n\t\r\"]" "" FUNC_GIT_COMMIT_DATE ${FUNC_GIT_COMMIT_DATE})
    execute_process(COMMAND date -d@${FUNC_GIT_COMMIT_DATE} +"%Y-%m-%d,%H:%M:%S"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        OUTPUT_VARIABLE FUNC_GIT_COMMIT_DATE)
    string (REGEX REPLACE "[\n\t\r]" "" FUNC_GIT_COMMIT_ID ${FUNC_GIT_COMMIT_ID})
    string (REGEX REPLACE "[\n\t\r]" "" FUNC_GIT_COMMIT_DATE ${FUNC_GIT_COMMIT_DATE})  
    set(${FUNC_GET_GIT_INFO_GIT_COMMIT_ID} ${FUNC_GIT_COMMIT_ID} PARENT_SCOPE)
    set(${FUNC_GET_GIT_INFO_GIT_COMMIT_DATE} ${FUNC_GIT_COMMIT_DATE} PARENT_SCOPE)
endfunction(GET_GIT_INFO)

#===================== option=====================
option(OPT_BUILD_BOOST " if system has boost lib then add boost" OFF)
option(OPT_BUILD_MEMCHECK "if test memory leaks then add memory check" OFF)
option(OPT_BUILD_CPPCHECK "if static check cpp" OFF)
option(OPT_PERFORMANCE OFF)
option(OPT_BUILD_DATE "bin has build datatime" OFF)
option(OPT_BUILD_UT "enable ut module" OFF)
option(OPT_BUILD_TOOLS "enable ut module" OFF)

#===================== sub comp opt=====================
option(OPT_BUILD_FUSE "" OFF)
option(OPT_BUILD_REST "" OFF)
option(OPT_BUILD_JNI "" ON)
option(OPT_BUILD_TCPSERVER "" OFF)
option(OPT_BUILD_STRUCT2JSON "" ON)

#===================== module op=====================
set (HVAE_MEMCHECK_CONTEXT OFF)
set (HAVE_PERFORMANCE_CONTEXT OFF)
set (HAVE_GIT_CONTEXT ON)
set (HAVE_CPPCHECK_CONTEXT OFF)
set (HAVE_BUILD_DATE_CONTEXT OFF)

set (HAVE_TEST_TOOLS_CONTEXT OFF)
set (HAVE_UT_CONTEXT OFF)
set (HAVE_FUSE_CONTEXT OFF)
set (HAVE_REST_CONTEXT OFF)
set (HAVE_JNI_CONTEXT OFF)
set (HAVE_TCPSERVER_CONTEXT OFF)
set (HAVE_STRUCT2JSON_CONTEXT OFF)


if (OPT_BUILD_UT)
    set (HAVE_UT_CONTEXT ON)
    message(NOTICE "BUILD UT IS ON")
endif(OPT_BUILD_UT)
if (OPT_BUILD_CPPCHECK)
    set (HAVE_CPPCHECK_CONTEXT ON)
    message(NOTICE "BUILD CPPCHECK IS ON")
endif(OPT_BUILD_CPPCHECK)
if (OPT_BUILD_MEMCHECK)
    set (HVAE_MEMCHECK_CONTEXT ON)
    message(NOTICE "BUILD MEMCHECK IS ON")
endif(OPT_BUILD_MEMCHECK)
if (OPT_PERFORMANCE)
    set (HAVE_PERFORMANCE_CONTEXT ON)
    message(NOTICE "BUILD PERFORMANCE IS ON")
endif(OPT_PERFORMANCE)

if (OPT_BUILD_FUSE)
    set (HAVE_FUSE_CONTEXT ON)
    message(NOTICE "BUILD FUSE IS ON")
endif (OPT_BUILD_FUSE)
if (OPT_BUILD_REST)
    set (HAVE_REST_CONTEXT ON)
    message(NOTICE "BUILD REST IS ON")
endif (OPT_BUILD_REST)
if (OPT_BUILD_JNI)
    set (HAVE_JNI_CONTEXT ON)
    message(NOTICE "BUILD JNI IS ON")
endif (OPT_BUILD_JNI)
if (OPT_BUILD_TCPSERVER)
    set (HAVE_TCPSERVER_CONTEXT ON)
    message(NOTICE "BUILD TCPSERVER IS ON")
endif (OPT_BUILD_TCPSERVER)
if (OPT_BUILD_STRUCT2JSON)
    set (HAVE_STRUCT2JSON_CONTEXT ON)
    message(NOTICE "BUILD STRUCT2JSON IS ON")
endif (OPT_BUILD_STRUCT2JSON)


#===================== set build flags =====================
if (HAVE_CPPCHECK_CONTEXT)
    message(NOTICE "BUILD CPPCHECK IS ON")
    find_program(CMAKE_C_CPPCHECK NAMES cppcheck)
    set(CMAKE_EXPORT_COMPILE_COMMANDS on)
    set(CPPCHECK_FLAG cppcheck --enable=warning --suppress=uninitMemberVar --suppress=uninitStructMember)
    list(APPEND CPPCHECK_FLAG --suppress=noOperatorEq --suppress=noCopyConstructor)
    list(APPEND CPPCHECK_FLAG --suppress=unknownMacro --suppress=nullPointerRedundantCheck)
    list(APPEND CPPCHECK_FLAG --suppress=uselessAssignmentPtrArg)
    list(APPEND CPPCHECK_FLAG --suppress=uninitMemberVarPrivate --suppress=uninitMemberVarPrivate --suppress=uninitMemberVar)
endif(HAVE_CPPCHECK_CONTEXT)

if (HVAE_MEMCHECK_CONTEXT)
    message(NOTICE "BUILD OPT_BUILD_MEMCHECK IS ON")
    #set (ENV{ASAN_OPTIONS} "detect_odr_violation=0:halt_on_error=0:detect_leaks=1:malloc_context_size=20:log_path=/tmp/libasan.log")
    set(SAN_FLAGS "${SAN_FLAGS} -fsanitize=address -fsanitize=leak -fsanitize-recover=address")
    set(SAN_FLAGS "${SAN_FLAGS} -fno-stack-protector -fno-omit-frame-pointer -static-libasan")
    set(LINK_SAN_FLAGS "${LINK_SAN_FLAGS} ${SAN_FLAGS}")
    set(SHARED_LINK_SAN_FLAGS "${SHARED_LINK_SAN_FLAGS} ${SAN_FLAGS}")
    
    #set(SAN_FLAGS "${SAN_FLAGS} -fsanitize=thread -static-libtsan")
    #set(LINK_SAN_FLAGS "${LINK_SAN_FLAGS} -fsanitize=thread -static-libtsan")
    #set(SHARED_LINK_SAN_FLAGS "${SHARED_LINK_SAN_FLAGS} ${LINK_SAN_FLAGS}")
    
    #set(SAN_FLAGS "${SAN_FLAGS} -fsanitize=memory -fsanitize-memory-track-origins -fno-optimize-sibling-calls -static-libmsan")
    #set(LINK_SAN_FLAGS "${LINK_SAN_FLAGS} -fsanitize=memory -static-libmsan")
    #set(SHARED_LINK_SAN_FLAGS "${SHARED_LINK_SAN_FLAGS} ${LINK_SAN_FLAGS}")

    #set(SAN_FLAGS "${SAN_FLAGS} -fsanitize=undefined -fno-sanitize-recover=all -static-libubsan")
    #set(LINK_SAN_FLAGS "${LINK_SAN_FLAGS} -fsanitize=undefined -static-libubsan")
    #set(SHARED_LINK_SAN_FLAGS "${SHARED_LINK_SAN_FLAGS} ${LINK_SAN_FLAGS}")

    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${SAN_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SAN_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINK_SAN_FLAGS}")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${SHARED_LINK_SAN_FLAGS}")

    set(ENV{ASAN_OPTIONS} "detect_odr_violation=0:halt_on_error=0:detect_leaks=1:malloc_context_size=20:log_path=/tmp/libasan.log")
    message(status "CMAKE_C_FLAGS=${CMAKE_C_FLAGS} CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS} CMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS} CMAKE_SHARED_LINKER_FLAGS=${CMAKE_SHARED_LINKER_FLAGS}")

endif (HVAE_MEMCHECK_CONTEXT)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DOS_BSD -DDEBUG )
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -ggdb -O0 -Wall ${C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -ggdb -O0 -Wall ${CXX_FLAGS}")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wall ${C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall ${CXX_FLAGS}")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-rpath=$ORIGIN")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-rpath=$ORIGIN")
if (HAVE_PERFORMANCE_CONTEXT)
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -pg")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pg")
endif(HAVE_PERFORMANCE_CONTEXT)

#version info
if (HAVE_GIT_CONTEXT)
    if (HAVE_BUILD_DATE_CONTEXT)
        execute_process(COMMAND date +"%Y-%m-%d,%H:%M:%S"
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            OUTPUT_VARIABLE FUNC_GET_GIT_INFO_GIT_BUILD_DATE)
        string (REGEX REPLACE "[\n\t\r\\]" "" PRO_BUILD_DATE
            ${PRO_BUILD_DATE})
        add_definitions(-DPRO_BUILD_DATE=${PRO_BUILD_DATE}
            -DGIT_BUILD_DATE="GIT_BUILD_DATE=\"${PRO_BUILD_DATE}\"")
    endif (HAVE_BUILD_DATE_CONTEXT)

    GET_GIT_INFO(CURRENT_PRO
        PRO_GIT_CODES_VERSION PRO_GIT_CODES_DATE)
    add_definitions(
        -DPRO_GIT_COMMIT=${PRO_GIT_CODES_VERSION}
        -DPRO_GIT_DATE=${PRO_GIT_CODES_DATE}
        -DSHOW_GIT_CODES_VERSION_STR="GIT_CODES_VERSION=\"${PRO_GIT_CODES_VERSION}\""
        -DSHOW_GIT_CODES_DATE_STR="GIT_CODES_DATE=\"${PRO_GIT_CODES_DATE}\"")

    if("$ENV{PRO_RELEASE_MAJOR}" STREQUAL "")
        set(PRO_RELEASE_MAJOR 0)
    else()
        set(PRO_RELEASE_MAJOR $ENV{PRO_RELEASE_MINOR})
    endif()
    if("$ENV{PRO_RELEASE_MINOR}" STREQUAL "")
        set(PRO_RELEASE_MINOR 0)
    else()
        set(PRO_RELEASE_MINOR $ENV{PRO_RELEASE_MINOR})
    endif()
    if("$ENV{PRO_RELEASE_BUILD}" STREQUAL "")
        set(PRO_RELEASE_BUILD 0)
    else()
        set(PRO_RELEASE_BUILD $ENV{PRO_RELEASE_BUILD})
    endif()
    set(PRO_RELEASE_VERSION "${PRO_RELEASE_MINOR}:${PRO_RELEASE_MINOR}:${PRO_RELEASE_BUILD}")
    
    add_definitions(
        -DPRO_RELEASE_MAJOR=${PRO_RELEASE_MINOR}
        -DPRO_RELEASE_MINOR=${PRO_RELEASE_MINOR}
        -DPRO_RELEASE_BUILD=${PRO_RELEASE_BUILD}
        -DPRO_RELEASE_VERSION="${PRO_RELEASE_VERSION}"
        -DSHOW_GIT_RELEASE_VERSION="GIT_RELEASE_VERSION=${PRO_RELEASE_VERSION}")
endif(HAVE_GIT_CONTEXT)
message(NOTICE "===================GET_GIT_INFO =========================\n"
    "INFO:\n" 
    " GIT_CODES_VERSION " ${PRO_GIT_CODES_VERSION}
    "\n; GIT_CODES_DATE " ${PRO_GIT_CODES_DATE}
    "\n;  PRO_RELEASE_MAJOR " ${PRO_RELEASE_MAJOR}
    "\n;  PRO_RELEASE_MINOR " ${PRO_RELEASE_MINOR}
    "\n;  PRO_RELEASE_BUILD " ${PRO_RELEASE_BUILD}
    "\n;  PRO_RELEASE_VERSION " ${PRO_RELEASE_VERSION})   

#===================== MIAN =====================
# --------- thirdparty projects ------------------

# --------- projects ------------------
#if (HAVE_FUSE_DEMO_CONTEXT)

#endif(HAVE_FUSE_DEMO_CONTEXT)
#if (HAVE_FUSE_DEMO_CONTEXT)

#endif(HAVE_FUSE_DEMO_CONTEXT)
#if (HAVE_FUSE_DEMO_CONTEXT)

#endif(HAVE_FUSE_DEMO_CONTEXT)

if (HAVE_JNI_CONTEXT)
	 add_subdirectory("jni")
endif(HAVE_JNI_CONTEXT)

if (HAVE_STRUCT2JSON_CONTEXT)
    add_subdirectory("struct2json")
endif(HAVE_STRUCT2JSON_CONTEXT)
# --------- tools projects ------------------
#if (HAVE_TEST_TOOLS_CONTEXT)
    #add_subdirectory("test")
#endif(HAVE_TEST_TOOLS_CONTEXT)

# ut
#if (HAVE_UT_CONTEXT)
    #add_subdirectory("unittest")
#endif(HAVE_UT_CONTEXT)


