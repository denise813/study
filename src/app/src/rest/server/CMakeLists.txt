
set(BIN_NAME httpd)

set(CURRENT_DIR ${TOP_DIR}/src/httpd)
set(THIRDPATRY_DIR ${TOP_DIR}/src/thridparty)

include_directories(${THIRDPATRY_DIR})
include_directories(${CURRENT_DIR}/include)
include_directories(${CURRENT_DIR}/src)


set(SRC_FILES ${CURRENT_DIR}/src/controller.cc
				${CURRENT_DIR}/src/demo_router.cc
				${CURRENT_DIR}/src/demo_handler.cc
				${CURRENT_DIR}/src/http_service_manager.cc
				${CURRENT_DIR}/src/root_controller.cc)
				
message(NOTICE "SRC_FILES=${SRC_FILES}")


add_executable(${BIN_NAME} ${CURRENT_DIR}/httpd_main.cc ${SRC_FILES})


install(TARGETS ${BIN_NAME} LIBRARY DESTINATION ${prefix}/bin)


