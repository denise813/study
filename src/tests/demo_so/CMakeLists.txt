############################################################################


############################################################################
# 设置源码
#--------------------------------------------------------------------------
#modlue hello_so
#set(SOURCES ./test_demo_so_main.cc)
include_directories(${TOPDIR}/src/demo_so)
set(hello_libs hello)
set(BIN_NAME demo_so)
add_executable(${BIN_NAME} ./test_demo_so_main.cc)
target_link_libraries(${BIN_NAME} ${hello_libs})
install(TARGETS demo_so LIBRARY DESTINATION ${prefix}/test/bin)


#--------------------------------------------------------------------------


