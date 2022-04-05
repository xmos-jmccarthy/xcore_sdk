// Copyright (c) 2022 XMOS LIMITED. This Software is subject to the terms of the
// XMOS Public License: Version 1

/* System headers */
#include <platform.h>
#include <xs1.h>
#include <xcore/channel.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"
#include "stream_buffer.h"
#include "queue.h"
#include "event_groups.h"

/* Library headers */
#include "rtos_printf.h"
#include "src.h"

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"
#include "usb_support.h"
#include "usb_audio.h"
#include "audio_pipeline/audio_pipeline.h"
#include "inference_engine.h"
#include "fs_support.h"
#include "gpio_ctrl/gpi_ctrl.h"
#include "keyword_inf_eng.h"
#include "inference_hmi/inference_hmi.h"

volatile int mic_from_usb = appconfMIC_SRC_DEFAULT;

void audio_pipeline_input(void *input_app_data,
                          int32_t **input_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) input_app_data;

    static int flushed;
    while (!flushed) {
        size_t received;
        received = rtos_mic_array_rx(mic_array_ctx,
                                     input_audio_frames,
                                     frame_count,
                                     0);
        if (received == 0) {
            rtos_mic_array_rx(mic_array_ctx,
                              input_audio_frames,
                              frame_count,
                              portMAX_DELAY);
            flushed = 1;
        }
    }

    /*
     * NOTE: ALWAYS receive the next frame from the PDM mics,
     * even if USB is the current mic source. The controls the
     * timing since usb_audio_recv() does not block and will
     * receive all zeros if no frame is available yet.
     */
    rtos_mic_array_rx(mic_array_ctx,
                      input_audio_frames,
                      frame_count,
                      portMAX_DELAY);

#if appconfUSB_ENABLED
    int32_t **usb_mic_audio_frame = NULL;

    if (mic_from_usb) {
        usb_mic_audio_frame = input_audio_frames;
    }

    /*
     * As noted above, this does not block.
     */
    usb_audio_recv(intertile_ctx,
                   frame_count,
                   usb_mic_audio_frame,
                   ch_count);
#endif
}

#include <xcore/hwtimer.h>
static uint32_t last_time = 0;

int audio_pipeline_output(void *output_app_data,
                          int32_t **output_audio_frames,
                          size_t ch_count,
                          size_t frame_count)
{
    (void) output_app_data;

    uint32_t now = get_reference_time();
    // rtos_printf("ref time is %d, diff %d\n", now, now-last_time);
    last_time = now;

#if appconfI2S_ENABLED
    /* I2S expects sample channel format */
    int32_t tmp[appconfAUDIO_PIPELINE_FRAME_ADVANCE][appconfAUDIO_PIPELINE_CHANNELS];
    for (int j=0; j<appconfAUDIO_PIPELINE_FRAME_ADVANCE; j++) {
        /* ASR output is first */
        tmp[j][0] = output_audio_frames[j];
        tmp[j][1] = output_audio_frames[j];
    }

    rtos_i2s_tx(i2s_ctx,
                (int32_t*) tmp,
                frame_count,
                portMAX_DELAY);
#endif

#if appconfUSB_ENABLED
    usb_audio_send(intertile_ctx,
                frame_count,
                output_audio_frames,
                4);
#endif

#if appconfINFERENCE_ENABLED
    inference_engine_sample_push((uint8_t *)output_audio_frames, frame_count);
#endif

    return AUDIO_PIPELINE_FREE_FRAME;
}

void vApplicationMallocFailedHook(void)
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    xassert(0);
    for(;;);
}

static void mem_analysis(void)
{
	for (;;) {
		rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

#include "xcore_clock_control.h"
void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(1)
    gpio_gpi_init(gpio_ctx_t0);
#endif

#if ON_TILE(FS_TILE_NO)
    rtos_fatfs_init(qspi_flash_ctx);
#endif

#if appconfINFERENCE_ENABLED && ON_TILE(INFERENCE_TILE_NO)
    keyword_engine_args_t *kw_eng_args = pvPortMalloc( sizeof(keyword_engine_args_t) );
    kw_eng_args->egrp_inference = xEventGroupCreate();
    inference_engine_create(appconfINFERENCE_MODEL_RUNNER_TASK_PRIORITY, kw_eng_args);
    inference_hmi_create(appconfINFERENCE_HMI_TASK_PRIORITY, kw_eng_args->egrp_inference);
#endif

#if ON_TILE(AUDIO_PIPELINE_TILE_NO)
    audio_pipeline_init(NULL, NULL);
#endif

#if ON_TILE(0)
    rtos_printf("switch ref is %d\n", get_local_node_switch_clk_div());
    // set_local_node_switch_clk_div(255);

    set_tile_processor_clk_div(get_local_tile_id(), 600);
    rtos_printf("tile[%d] clock rate %d\n", THIS_XCORE_TILE, get_local_tile_processor_clock());
    rtos_printf("ref is %d\n", get_local_ref_clock());


    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000));
        rtos_printf("1s\n");
    }
#endif

#if ON_TILE(1)
    // set_tile_processor_clk_div(get_local_tile_id(), 2); // 600/2 = 300
    // set_tile_processor_clk_div(get_local_tile_id(), 3); // 600/3 = 200 // lowest i2s functions
    set_tile_processor_clk_div(get_local_tile_id(), 5); // 600/5 = 120
    // set_tile_processor_clk_div(get_local_tile_id(), 6); // 600/6 = 100
    // set_tile_processor_clk_div(get_local_tile_id(), 12); // 600/12 = 50
    // set_tile_processor_clk_div(get_local_tile_id(), 24); // 600/24 = 25
    // set_tile_processor_clk_div(get_local_tile_id(), 600); // 600/600 = 1
    rtos_printf("tile[%d] clock rate %d\n", THIS_XCORE_TILE, get_local_tile_processor_clock());
#endif

    mem_analysis();
    /*
     * TODO: Watchdog?
     */
}

void vApplicationMinimalIdleHook(void)
{
    rtos_printf("idle hook on tile %d core %d\n", THIS_XCORE_TILE, rtos_core_id_get());
    asm volatile("waiteu");
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

#if appconfUSB_ENABLED && ON_TILE(USB_TILE_NO)
    usb_audio_init(intertile_ctx, appconfUSB_AUDIO_TASK_PRIORITY);
#endif

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("tile clock rate %d\n", get_local_tile_processor_clock());

    unsigned pre_div = 0;
    unsigned mul = 0;
    unsigned post_div = 0;
    get_node_pll_ratio(get_local_tile_id(), &pre_div, &mul, &post_div);
    rtos_printf("pll ratios: pre %d mul %d post %d\n", pre_div, mul, post_div);

    rtos_printf("proc clk div %d\n", get_tile_processor_clk_div(get_local_tile_id()));
    enable_local_tile_processor_clock_divider();

    rtos_printf("tile clock rate %d\n", get_local_tile_processor_clock());
    set_tile_processor_clk_div(get_local_tile_id(), 1);
    rtos_printf("proc clk div %d\n", get_tile_processor_clk_div(get_local_tile_id()));

    rtos_printf("tile clock rate %d\n", get_local_tile_processor_clock());

    rtos_printf("ref div %d\n",get_local_node_ref_clk_div());

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c0;
    (void) c2;
    (void) c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3)
{
    (void) c1;
    (void) c2;
    (void) c3;

    tile_common_init(c0);
}
#endif
