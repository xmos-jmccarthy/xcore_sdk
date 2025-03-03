cmake_minimum_required(VERSION 3.20)

set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
message(STATUS "Using SDK at ${XCORE_SDK_PATH}")

#**********************
# Set up some custom vars
#**********************
string(ASCII 27 ESC)
set(COLOR_RESET   "${ESC}[m")
set(COLOR_RED     "${ESC}[31m")
set(COLOR_GREEN   "${ESC}[32m")
set(COLOR_YELLOW  "${ESC}[33m")
set(COLOR_BLUE    "${ESC}[34m")
set(COLOR_MAGENTA "${ESC}[35m")
set(COLOR_CYAN    "${ESC}[36m")
set(COLOR_WHITE   "${ESC}[37m")

#********************************
# Set up compiler
#********************************
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_toolchain.cmake")

#********************************
# Set up hardware target
#********************************
if(NOT DEFINED BOARD)
    message(FATAL_ERROR "BOARD must be defined to specify the hardware target.")
endif()
include("${XCORE_SDK_PATH}/tools/cmake_utils/board_support/${BOARD}.cmake")

#********************************
# Set up for multi-tile builds
#********************************
option(MULTITILE_BUILD "Enable to build multitile RTOS" FALSE)
if(MULTITILE_BUILD)
    include("${XCORE_SDK_PATH}/tools/cmake_utils/multitile_build.cmake")
endif()

#**********************
# Paths
#**********************
set(MODULES_DIR "${XCORE_SDK_PATH}/modules")

#********************************
# Gather various sources
#********************************
include("${MODULES_DIR}/hil/hil.cmake")
include("${MODULES_DIR}/rtos/rtos.cmake")
include("${MODULES_DIR}/thirdparty/thirdparty.cmake")
include("${MODULES_DIR}/modules.cmake")

#**********************
# set user variables
#**********************
set(XMOS_PLATFORM_SOURCES
    ${HIL_SOURCES}
    ${MODULES_SOURCES}
    ${THIRDPARTY_SOURCES}
)

set(XMOS_PLATFORM_INCLUDES
    ${HIL_INCLUDES}
    ${MODULES_INCLUDES}
    ${THIRDPARTY_INCLUDES}
)

list(REMOVE_DUPLICATES XMOS_PLATFORM_SOURCES)
list(REMOVE_DUPLICATES XMOS_PLATFORM_INCLUDES)

set(XMOS_RTOS_PLATFORM_SOURCES
    ${XMOS_PLATFORM_SOURCES}
    ${RTOS_SOURCES}
)

set(XMOS_RTOS_PLATFORM_INCLUDES
    ${XMOS_PLATFORM_INCLUDES}
    ${RTOS_INCLUDES}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_INCLUDES)

set(XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES
    ${XMOS_RTOS_PLATFORM_SOURCES}
    ${RTOS_NETWORKING_SOURCES}
)

set(XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES
    ${XMOS_RTOS_PLATFORM_INCLUDES}
    ${RTOS_NETWORKING_INCLUDES}
)

list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES)
list(REMOVE_DUPLICATES XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES)
