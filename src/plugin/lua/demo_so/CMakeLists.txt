############################################################################
set(LIB_NAME hello)
set(COMM_DIR ${TOP_DIR}/src/common)
set(SRC_DIR ${TOP_DIR}/src)

message(NOTICE "COMM_DIR="${COMM_DIR})


include_directories(${COMM_DIR})
include_directories(${SRC_DIR})


set(CURRENT_DIR ${TOP_DIR}/src/plugin/lua/demo_so)
set(SRC_FILES ${CURRENT_DIR}/lua_call_hello_so.cc
				${COMM_DIR}/forker/forker.cc)
				

#find_library (${LIB_NAME} PATHS ${datadir}./mylib REQUIRED)
message(NOTICE "SRC_FILES=${SRC_FILES}")
add_library(${LIB_NAME} SHARED ${SRC_FILES})

target_link_libraries(${LIB_NAME} dl lua-5.1)
target_link_libraries(${LIB_NAME} demo)

install(TARGETS ${LIB_NAME} LIBRARY DESTINATION ${prefix}/lib)

