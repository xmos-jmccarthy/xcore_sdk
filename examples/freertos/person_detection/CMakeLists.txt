cmake_minimum_required(VERSION 3.20)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(MULTITILE_BUILD TRUE)
set(USE_AIF TRUE)
set(USE_DISPATCHER TRUE)

#**********************
# Get path to XCore SDK
#**********************
set(XCORE_SDK_PATH "${CMAKE_CURRENT_LIST_DIR}")
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)
cmake_path(GET XCORE_SDK_PATH PARENT_PATH XCORE_SDK_PATH)

## Import XMOS configurations
include("${XCORE_SDK_PATH}/tools/cmake_utils/xmos_platform.cmake")

## Define project
project(person_detect)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-Wxcore-fptrgroup"
    "-Wno-unknown-pragmas"
    "-mcmodel=large"
    "${CMAKE_CURRENT_SOURCE_DIR}/app/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "app/main.c"
    "app/board_init.c"
    "app/ov2640/ov2640.c"
    "app/ov2640/ov2640_progs.c"
    "app/spi_camera/spi_camera.c"
    "app/model_runner/person_detect_model_runner.cc"
    "app/model_runner/person_detect_model_data.c"
    "app/model_runner/person_detect_task.c"
    "${XCORE_SDK_PATH}/modules/device_memory_support/rtos/xcore_device_memory.c"
    ${XMOS_RTOS_PLATFORM_SOURCES}
)
set(APP_INCLUDES
    "app"
    "app/ov2640"
    "app/spi_camera"
    "app/model_runner"
    "${XCORE_SDK_PATH}/modules/device_memory_support/rtos"
	${XMOS_RTOS_PLATFORM_INCLUDES}
)

add_compile_definitions(
    OUTPUT_IMAGE_STREAM=1
    DEBUG_PRINT_ENABLE=1
    TF_LITE_STATIC_MEMORY=1
    XCORE=1
)

if(${USE_EXTMEM})
    add_compile_definitions(USE_EXTMEM=1)
endif()

## Set any additional flags only for C++
set(CMAKE_CXX_FLAGS  "-std=c++11")

set(RTOS_TILE_LIST 0 1 )
create_multitile_target(RTOS_TILE_LIST)