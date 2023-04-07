cmake_minimum_required(VERSION 3.10.2)
project (Demo)

#########################################################################
#平台检查
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(SYSTEM_NAME_LINUX ON)
  #FIND_PACKAGE(Threads)
endif(CMAKE_SYSTEM_NAME MATCHES "Linux")

if(WIN32)
  # The Windows headers (e.g. coming from mingw or the Windows SDK) check
  # the targeted Windows version. The availability of certain functions and
  # structures will depend on it.
  set(SYSTEM_NAME_WIN32_WINNT "0x0A00" CACHE STRING "Targeted Windows version.")
  add_definitions(-D_WIN32_WINNT=${WIN32_WINNT})
endif()

#########################################################################
# 输出目录修改
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
#set(CURRENT_DIR ${})


set(CMAKE_CXX_STANDARD 14)

#########################################################################
#set (Tutorial_VERSION_MAJOR 1)
#set (Tutorial_VERSION_MINOR 0)
# 执行命令获取git 信息
execute_process(COMMAND	git log -1 --format="%H" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_VERSION)
execute_process(COMMAND	git log -1 --format="%ct" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_TIME)
string (REGEX REPLACE "[\n\t\r\"]" "" GIT_TIME ${GIT_TIME})
execute_process(COMMAND date -d@${GIT_TIME} +"%Y-%m-%d %H:%M:%S" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE GIT_DATETIME)
execute_process(COMMAND date +"%Y-%m-%d %H:%M:%S" WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} OUTPUT_VARIABLE BUILD_DATETIME)
# 对得到的结果进行处理，尤其注意 \n,\t,\r之类的特殊字符，在cmake时没问题，但是生成的.cmake文件有问题，导致make出错。
string (REGEX REPLACE "[\n\t\r]" "" GIT_VERSION ${GIT_VERSION})
string (REGEX REPLACE "[\n\t\r]" "" GIT_DATETIME ${GIT_DATETIME})
string (REGEX REPLACE "[\n\t\r]" "" BUILD_DATETIME ${BUILD_DATETIME})
message(NOTICE "git版本:" ${GIT_VERSION} ";  git日期:" ${GIT_DATETIME} ";  编译日期:" ${BUILD_DATETIME})

add_definitions( -DGIT_VERSION=${GIT_VERSION})
add_definitions( -DGIT_DATETIME=${GIT_DATETIME})
add_definitions( -DBUILD_DATETIME=${BUILD_DATETIME})

########################################################################
# 定义全局变量
set(TOPDIR ${PROJECT_SOURCE_DIR})
#set(INSTALL_DATADIR ${TOPDIR})
#########################################################################
# 增加编译选项，把宏导入源码
option(WITH_DEBUG "debug flag." ON)
if(WITH_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
endif()
#set(CMAKE_BUILD_TYPE Release)

#######################################################################
# 外部库配置
#--------------------------------------------------------------------


########################################################################
# src 配置
add_subdirectory(src)


############################################################################
# 测试 配置
#-------------------------
# 测试框架
option(WITH_TESTS "enable the build of test package" OFF)
if (WITH_TESTS)
  set(CMAKE_CTEST_COMMAND ctest)
endif(WITH_TESTS)

############################################################################
# 其他依赖工具

#-------------------------
# 代码缺陷静态检查工具
#find_package(CppCheck)
#-----------------------
#去除多余头文件
#find_package(IWYU)

