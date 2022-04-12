
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
