###########################################################################
set(CMAKE_CXX_STANDARD 14)

# set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_definitions(-DOS_BSD -DDEBUG)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
endif()

############################################################################
# 设置目录
set(bindir ${CMAKE_INSTALL_FULL_BINDIR})
set(sbindir ${CMAKE_INSTALL_FULL_SBINDIR})
set(libdir ${CMAKE_INSTALL_FULL_LIBDIR})
set(sysconfdir ${CMAKE_INSTALL_FULL_SYSCONFDIR})
set(libexecdir ${CMAKE_INSTALL_FULL_LIBEXECDIR})
set(pkgdatadir ${CMAKE_INSTALL_FULL_DATADIR})
set(datadir ${INSTALL_DATADIR})
set(prefix ${CMAKE_INSTALL_PREFIX})
#CMAKE_CURRENT_SOURCE_DIR

############################################################################
set(HAVE_BOOST_CONTEXT OFF)
set(HAVE_VDDK_CONTEXT OFF)
set(HAVE_PY_CONTEXT OFF)
set(HAVE_GLIB_CONTEXT OFF)
set (HAVE_LIST_HEAD_CONTEXT OFF)
set(HAVE_TEST_CONTEXT ON)

#########################################################################
# 设置宏定义

if(SYSTEM_NAME_LINUX)
  add_definitions("-D_GNU_SOURCE")
endif()

if (HAVE_BOOST_CONTEXT)
    MESSAGE(STATUS "define boost")
    add_definitions("-DBOOST_LOG_WITHOUT_CHAR -DBOOST_LOG_WITHOUT_WCHAR_T")
	add_definitions("-DBOOST_LOG_WITHOUT_SYSLOG -DBOOST_LOG_USE_STD_REGEX")
endif(HAVE_BOOST_CONTEXT)

########################################################################
# 设置编译选项
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wtype-limits -Wignored-qualifiers")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Werror=format-security")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winit-self -Wno-unknown-pragmas")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-strict-aliasing -fsigned-char")

#if(SYSTEM_NAME_LINUX)
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wtype-limits -Wignored-qualifiers")
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wpointer-arith -Werror=format-security")
#  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Winit-self -Wno-unknown-pragmas")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unknown-pragmas")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wnon-virtual-dtor")
#  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-ignored-qualifiers")
#  FIND_PACKAGE(Threads)
# endif()

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
#SET(DCMAKE_EXPORT_COMPILE_COMMANDS ON)

########################################################################
# 设置依赖

#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads)
#MESSAGE(STATUS "THREADS_INCLUDE_DIRS = ${CMAKE_THREAD_LIBS_INIT}")
#set(SYSLIB)
#list(APPEND SYSLIB ${THREADS_INCLUDE_DIRS})
#MESSAGE(STATUS "SYSLIB = ${SYSLIB}")
#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
#find_package(Threads REQUIRED)

#------------------------------------------------------------------
# 第三方库
if (HAVE_BOOST_CONTEXT)
	#set(BOOST_COMPONENTS)
	#list(BOOST_COMPONENTS APPEND atomic chrono thread system)
	#list(BOOST_COMPONENTS APPEND regex random program_options date_time)
	#list(BOOST_COMPONENTS APPEND iostreams log)
	#set(BOOST_HEADER_COMPONENTS container)

	set(Boost_USE_STATIC_LIBS ON)
	set(Boost_USE_MULTITHREADED OFF)
	set(Boost_USE_STATIC_RUNTIME ON)
	set(BOOST_NO_SYSTEM_PATHS ON)
	set(BOOST_ROOT ./thridparty/boost_1_80_0/liboost)

	#查找boost库
	SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} ${BOOST_ROOT})
	#find_package(Boost REQUIRED COMPONENTS system log)
	if (HAVE_BOOST_LOG_CONTEXT) 
		find_package(Boost REQUIRED log)
	else()
		find_package(Boost)
	endif(HAVE_BOOST_LOG_CONTEXT)

	MESSAGE(STATUS "Boost_INCLUDE_DIRS = ${Boost_INCLUDE_DIRS}")
	MESSAGE(STATUS "Boost_LIBRARIES = ${Boost_LIBRARIES}")
	MESSAGE(STATUS "Boost_LIB_VERSION = ${Boost_LIB_VERSION}")
	MESSAGE(STATUS "Boost_LIBRARY_DIRS = ${Boost_LIBRARY_DIRS}")
	include_directories(BEFORE SYSTEM ${Boost_INCLUDE_DIRS})
	link_directories(${Boost_LIBRARY_DIRS})
	set(BOOST_LIBS)
	set(BOOST_LOG_LIB libboost_log)
	MESSAGE(STATUS "BOOST_LIBS = ${BOOST_LIBS}")
endif(HAVE_BOOST_CONTEXT)

if (HAVE_PY_CONTEXT)
    find_package(Python3 COMPONENTS Interpreter Development REQUIRED)
    set(PY_INCLUDE_DIR)
    list(APPEND PY_INCLUDE_DIR ${Python3_INCLUDE_DIRS})
    set(PY_LIB_DIR)
    list(APPEND PY_LIB_DIR ${Python3_LIBRARIES})
    MESSAGE(STATUS "PY_INCLUDE_DIR = " ${PY_INCLUDE_DIR})
    MESSAGE(STATUS "PY_LIB_DIR = " ${PY_LIB_DIR})
    include_directories(SYSTEM ${PY_INCLUDE_DIR})
    link_directories(${PY_LIB_DIR})
    set(PY_LIBS)
    list(APPEND PY_LIBS python3.6m)
endif(HAVE_PY_CONTEXT)


############################################################################
# 设置源码
#--------------------------------------------------------------------------
#基本库


#--------------------------------------------------------------------------
#组件
# modlue hello_so
add_subdirectory(demo_so)
add_subdirectory(plugin/lua/demo_so)



# modlue httpd
add_subdirectory(httpd)

#--------------------------------------------------------------------------
# 设置测试代码
if (HAVE_TEST_CONTEXT)
    add_subdirectory(tests)
endif(HAVE_TEST_CONTEXT)


