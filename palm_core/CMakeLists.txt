cmake_minimum_required(VERSION 3.16.3)

project(palm_core VERSION "1.0.0" LANGUAGES C CXX)

if (WIN32)
	set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")
elseif (UNIX)
	set(ULTRALEAP_PATH_ROOT "/usr/share/doc/ultraleap-hand-tracking-service/lib/cmake/LeapSDK")
endif()

find_package(LeapSDK
	5
	REQUIRED
	PATHS
		"${ULTRALEAP_PATH_ROOT}")

if (UNIX)    
    find_package(Threads REQUIRED)    
endif (UNIX)

add_executable(
	palm_core
	"main.cpp" 
	"lib/LeapC.h"
	"leap/LeapConnection.cpp" "leap/LeapConnection.hpp"
	"tracking/Tracking.hpp"
	"tracking/Tracking.cpp")

target_link_libraries(
	palm_core
	PRIVATE
		LeapSDK::LeapC)

get_target_property(
	LEAPC_IMPORTED_CONFIG
	LeapSDK::LeapC
	IMPORTED_CONFIGURATIONS
)

get_target_property(
	LEAPC_SHARED_LIB_PATH
	LeapSDK::LeapC
	IMPORTED_LOCATION_${LEAPC_IMPORTED_CONFIG}
)

add_custom_command(
	TARGET
		palm_core
	POST_BUILD
	COMMAND
		${CMAKE_COMMAND} -E copy
		${LEAPC_SHARED_LIB_PATH}
		$<TARGET_FILE_DIR:palm_core>)


if (UNIX)
	target_link_libraries(
		palm_core
		PRIVATE
		Threads::Threads)
endif()    
