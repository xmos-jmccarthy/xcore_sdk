cmake_minimum_required(VERSION 3.20)

# Disable in-source build.
if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_BINARY_DIR}")
    message(FATAL_ERROR "In-source build is not allowed! Please specify a build folder.\n\tex:cmake -B build")
endif()

## Specify configuration
set(MULTITILE_BUILD TRUE)
set(USE_WIFI_MANAGER TRUE)
set(USE_DHCPD TRUE)
set(USE_FATFS TRUE)
set(USE_HTTP_CORE TRUE)
set(USE_JSON_PARSER TRUE)
set(USE_MQTT TRUE)
set(USE_SNTPD TRUE)
set(USE_TLS_SUPPORT TRUE)
set(USE_TRACE FALSE)
set(USE_CUSTOM_MBEDTLS_CONFIG TRUE)

## This app only supports the XCORE-AI-EXPLORER board
set(BOARD XCORE-AI-EXPLORER)

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
project(explorer_board)

## Enable languages for project
enable_language(CXX C ASM)

## Set XMOS application variables
set(APP_COMPILER_FLAGS
    "-Os"
    "-g"
    "-report"
    "-fxscope"
    "-lquadspi"
    "-mcmodel=large"
    "-Wno-xcore-fptrgroup"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/config.xscope"
    "${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}.xn"
)

set(APP_SOURCES
    "src/main.c"
    "src/platform/aic3204.c"
    "src/platform/app_pll_ctrl.c"
    "src/platform/driver_instances.c"
    "src/platform/platform_init.c"
    "src/platform/platform_start.c"
    "src/audio_pipeline/audio_pipeline.c"
    "src/example_pipeline/example_pipeline.c"
    "src/gpio_ctrl/gpio_ctrl.c"
    "src/network_demos/network_setup.c"
    "src/queue_to_tcp_stream/queue_to_tcp_stream.c"
    "src/thruput_test/thruput_test.c"
    "src/udp_cli/cli_rpc.c"
    "src/udp_cli/CLI-commands.c"
    "src/udp_cli/FreeRTOS_CLI.c"
    "src/udp_cli/UDPCommandServer.c"
    "src/tls_echo_demo/tls_echo_demo.c"
    "src/tls_echo_server/tls_echo_server.c"
    "src/http_demo/http_demo.c"
    "src/mqtt_demo/mqtt_demo_client.c"
    "src/mem_analysis/mem_analysis.c"
    ${XMOS_RTOS_PLATFORM_WITH_NETWORKING_SOURCES}
)

set(APP_INCLUDES
    "src"
    "src/udp_cli"
    ${XMOS_RTOS_PLATFORM_WITH_NETWORKING_INCLUDES}
)

add_compile_definitions(
    HTTP_DO_NOT_USE_CUSTOM_CONFIG=1
    DEBUG_PRINT_ENABLE=1
    DEBUG_PRINT_ENABLE_IOT_WIFI=1
    DEBUG_PRINT_ENABLE_WIFI_CONN_MGR=1
    DEBUG_PRINT_ENABLE_MQTT_DEMO_CLIENT=1
    MIC_DUAL_ENABLED=1
    MQTT_TASK=1
    DEBUG_PRINT_ENABLE_LIB_SOC_SW_WIFI=1
    SL_WFX_DEBUG_MASK=\(SL_WFX_DEBUG_ERROR|SL_WFX_DEBUG_INIT\)
    MQTTCLIENT_PLATFORM_HEADER=MQTTFreeRTOS.h
    MBEDTLS_CONFIG_FILE=\"mbedtls_sample_config.h\"
)

set(RTOS_TILE_LIST 0 1 )
create_multitile_target(RTOS_TILE_LIST)
