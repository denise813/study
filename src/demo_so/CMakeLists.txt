############################################################################
set(LIB_NAME demo)
set(CURRENT_DIR ${TOP_DIR}/src/demo_so)


set(DEMO_SO_FILES ${CURRENT_DIR}/src/demo_so.cc)
add_library(${LIB_NAME} SHARED ${DEMO_SO_FILES})


install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${prefix}/lib)

