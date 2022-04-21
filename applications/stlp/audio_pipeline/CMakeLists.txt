
## Create custom stlp audiopipelines
add_library(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms INTERFACE)
target_sources(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
        src/fixed_delay/audio_pipeline_t0.c
        src/fixed_delay/audio_pipeline_t1.c
        src/fixed_delay/aec/aec_process_frame_1thread.c
)
target_include_directories(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
        api
        src/fixed_delay
)
target_link_libraries(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
        sdk::core
        sdk::rtos_freertos
        sdk::rtos::sw_services::generic_pipeline
        avona::adec
        avona::aec
        avona::agc
        avona::ic
        avona::ns
        avona::vad
)
target_compile_definitions(xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms
    INTERFACE
)

## Create an alias
add_library(sdk::app::stlp::ap::fixeddelay ALIAS xcore_sdk_app_stlp_audio_pipeline_fixed_delay_aec_2x_2y_no_comms)



## Create custom stlp audiopipelines
add_library(xcore_sdk_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms INTERFACE)
target_sources(xcore_sdk_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
        src/adec/audio_pipeline_t0.c
        src/adec/audio_pipeline_t1.c
        src/adec/aec/aec_process_frame_1thread.c
        src/adec/stage_1.c
        src/adec/delay_buffer.c
)
target_include_directories(xcore_sdk_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
        api
        src/adec
        src/adec/aec
)
target_link_libraries(xcore_sdk_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
        sdk::core
        sdk::rtos_freertos
        sdk::rtos::sw_services::generic_pipeline
        avona::adec
        avona::aec
        avona::agc
        avona::ic
        avona::ns
        avona::vad
)
target_compile_definitions(xcore_sdk_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms
    INTERFACE
)

## Create an alias
add_library(sdk::app::stlp::ap::adec ALIAS xcore_sdk_app_stlp_audio_pipeline_adec_aec_2x_2y_no_comms)



## Create custom stlp audiopipelines
add_library(xcore_sdk_app_stlp_audio_pipeline_dummy INTERFACE)
target_sources(xcore_sdk_app_stlp_audio_pipeline_dummy
    INTERFACE
        src/none/audio_pipeline.c
)
target_include_directories(xcore_sdk_app_stlp_audio_pipeline_dummy
    INTERFACE
        api
        src/none
)
target_link_libraries(xcore_sdk_app_stlp_audio_pipeline_dummy
    INTERFACE
        sdk::core
        sdk::rtos_freertos
        sdk::rtos::sw_services::generic_pipeline
)
target_compile_definitions(xcore_sdk_app_stlp_audio_pipeline_dummy
    INTERFACE
)

## Create an alias
add_library(sdk::app::stlp::ap::dummy ALIAS xcore_sdk_app_stlp_audio_pipeline_dummy)
