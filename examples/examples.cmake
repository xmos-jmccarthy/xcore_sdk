## XCORE_XS3A only examples
if(${CMAKE_SYSTEM_NAME} STREQUAL XCORE_XS3A)
    ## Bare metal examples
    include(${CMAKE_CURRENT_LIST_DIR}/bare-metal/explorer_board/explorer_board.cmake)

    ## FreeRTOS examples
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/device_control/device_control.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/dispatcher/dispatcher.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/explorer_board/explorer_board.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/getting_started/getting_started.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/iot/iot.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/l2_cache/l2_cache.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/usb/usb.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/xlink/xlink.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/freertos/flash_benchmark/flash_benchmark.cmake)
else()
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/freertos/device_control/host)
endif()
