cmake_minimum_required(VERSION 3.16.3)

project(palm VERSION "1.0.0" LANGUAGES C CXX)


if (WIN32)
	#########################################
	# Build palm_proxy (only on windows)
	#########################################
	set(ULTRALEAP_PATH_ROOT "$ENV{ProgramFiles}/Ultraleap")

	find_package(LeapSDK
	5
	REQUIRED
	PATHS
		"${ULTRALEAP_PATH_ROOT}")

	add_executable(
		palm_proxy
		"palm_proxy.cpp" 
		"leap/LeapC.h"
		"leap/LeapConnection.cpp" "leap/LeapConnection.hpp"
		"tracking/Tracking.hpp"
		"proxy/ProxyConnection.h" "proxy/ProxyConnection.cpp")

	target_link_libraries(
		palm_proxy
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
			palm_proxy
		POST_BUILD
		COMMAND
			${CMAKE_COMMAND} -E copy
			${LEAPC_SHARED_LIB_PATH}
			$<TARGET_FILE_DIR:palm_proxy>)
endif()

if(UNIX)
	#########################################
	# Build palm_core (only on unix)
	#########################################
	find_package(Threads REQUIRED)
	#find_package(Boost COMPONENTS program_options REQUIRED)
	#SET(Boost_INCLUDE_DIR "/usr/include/boost/")
	#include_directories(${Boost_INCLUDE_DIRS})
	#link_directories(/usr/local/lib/)
	
	add_executable(
		palm_core
		"state/Utilities.hpp"
		"tracking/Tracking.hpp"
		"state/RobotState.hpp" "state/RobotState.cpp"
		"state/Obstacles.hpp"
		"state/Planning.hpp" "state/Planning.cpp"
		"state/Scene.hpp" "state/Scene.cpp"
		"palm_core.cpp" 
		)

	set_property(TARGET palm_core PROPERTY CXX_STANDARD 17)


	target_link_libraries(palm_core PRIVATE Threads::Threads)

	#target_link_libraries(palm_core LINK_PUBLIC ${Boost_LIBRARIES})

endif()


