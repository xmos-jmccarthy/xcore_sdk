// Copyright 2019-2022 XMOS LIMITED.
// This Software is subject to the terms of the XMOS Public Licence: Version 1.

/* System headers */
#include <platform.h>
#include <xs1.h>

/* FreeRTOS headers */
#include "FreeRTOS.h"
#include "queue.h"

/* Library headers */

/* App headers */
#include "app_conf.h"
#include "platform/platform_init.h"
#include "platform/driver_instances.h"

#define VERBOSE 1

void vApplicationMallocFailedHook( void )
{
    rtos_printf("Malloc Failed on tile %d!\n", THIS_XCORE_TILE);
    for(;;);
}

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    rtos_printf("\nStack Overflow!!! %d %s!\n", THIS_XCORE_TILE, pcTaskName);
    configASSERT(0);
}

static void print_info(uint32_t start, uint32_t end, uint32_t cnt)
{
    rtos_printf("\t%d bytes/us\n", cnt/((end-start)/100));
    rtos_printf("\t%d ticks/byte\n", (end-start)/cnt);
#if VERBOSE
    rtos_printf("\t\tDuration: %d ticks %dus Bytes: %d\n", (end - start), (end - start)/100, cnt);
#endif
}

#if ON_TILE(0)
static void test_sequential_read(uint32_t cnt) {
    uint8_t *buf = pvPortMalloc(sizeof(uint8_t) * cnt);
    configASSERT(buf);
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t start = 0;
    uint32_t end = 0;

    rtos_printf("Sequential read:\n");
    start = hwtimer_get_time(tmr);
    rtos_qspi_flash_read(qspi_flash_ctx, buf, 0, cnt);
    end = hwtimer_get_time(tmr);
    print_info(start, end, cnt);
    rtos_printf("\n");

    hwtimer_free(tmr);
    vPortFree(buf);
}

static void test_nonsequential_read(uint32_t cnt) {
    uint8_t *buf = pvPortMalloc(sizeof(uint8_t) * cnt);
    configASSERT(buf);
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t start = 0;
    uint32_t end = 0;

    rtos_printf("Non Sequential read:\n");
    start = hwtimer_get_time(tmr);
    for(int i=0; i<cnt; i++) {
        rtos_qspi_flash_read(qspi_flash_ctx, buf, 0, 1);
    }
    end = hwtimer_get_time(tmr);
    print_info(start, end, cnt);
    rtos_printf("\n");

    hwtimer_free(tmr);
    vPortFree(buf);
}

static void test_sequential_write(uint32_t cnt) {
    uint8_t *buf = pvPortMalloc(sizeof(uint8_t) * cnt);
    configASSERT(buf);
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t start = 0;
    uint32_t end = 0;

    rtos_printf("Sequential write:\n");
    start = hwtimer_get_time(tmr);
    rtos_qspi_flash_write(qspi_flash_ctx, buf, 0, cnt);
    end = hwtimer_get_time(tmr);
    print_info(start, end, cnt);
    rtos_printf("\n");

    hwtimer_free(tmr);
    vPortFree(buf);
}

static void test_nonsequential_write(uint32_t cnt) {
    uint8_t *buf = pvPortMalloc(sizeof(uint8_t) * cnt);
    configASSERT(buf);
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t start = 0;
    uint32_t end = 0;

    rtos_printf("Non Sequential write:\n");
    start = hwtimer_get_time(tmr);
    for(int i=0; i<cnt; i++) {
        rtos_qspi_flash_write(qspi_flash_ctx, buf, i, 1);
    }
    end = hwtimer_get_time(tmr);
    print_info(start, end, cnt);
    rtos_printf("\n");

    hwtimer_free(tmr);
    vPortFree(buf);
}

static void test_large_erase(uint32_t cnt) {
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t start = 0;
    uint32_t end = 0;

    rtos_printf("Large erase:\n");
    start = hwtimer_get_time(tmr);
    rtos_qspi_flash_erase(qspi_flash_ctx, 0, cnt);
    end = hwtimer_get_time(tmr);
    print_info(start, end, cnt);
    rtos_printf("\n");

    hwtimer_free(tmr);
}

static void test_small_erase(uint32_t cnt) {
    hwtimer_t tmr = hwtimer_alloc();
    uint32_t start = 0;
    uint32_t end = 0;

    rtos_printf("Small erases:\n");
    start = hwtimer_get_time(tmr);
    for(int i=0; i<cnt; i++) {
        rtos_qspi_flash_erase(qspi_flash_ctx, i, 1);
    }
    end = hwtimer_get_time(tmr);
    print_info(start, end, cnt);
    rtos_printf("\n");

    hwtimer_free(tmr);
}
#endif

void startup_task(void *arg)
{
    rtos_printf("Startup task running from tile %d on core %d\n", THIS_XCORE_TILE, portGET_CORE_ID());

    platform_start();

#if ON_TILE(0)
    test_sequential_read(1000);
    test_nonsequential_read(1000);
    test_sequential_write(1000);
    test_nonsequential_write(1000);
    test_large_erase(100);
    test_small_erase(100);
#endif

#if ON_TILE(1)
#endif

	for (;;) {
		// rtos_printf("Tile[%d]:\n\tMinimum heap free: %d\n\tCurrent heap free: %d\n", THIS_XCORE_TILE, xPortGetMinimumEverFreeHeapSize(), xPortGetFreeHeapSize());
		vTaskDelay(pdMS_TO_TICKS(5000));
	}
}

static void tile_common_init(chanend_t c)
{
    platform_init(c);
    chanend_free(c);

    xTaskCreate((TaskFunction_t) startup_task,
                "startup_task",
                RTOS_THREAD_STACK_SIZE(startup_task),
                NULL,
                appconfSTARTUP_TASK_PRIORITY,
                NULL);

    rtos_printf("start scheduler on tile %d\n", THIS_XCORE_TILE);
    vTaskStartScheduler();
}

#if ON_TILE(0)
void main_tile0(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c0;
    (void)c2;
    (void)c3;

    tile_common_init(c1);
}
#endif

#if ON_TILE(1)
void main_tile1(chanend_t c0, chanend_t c1, chanend_t c2, chanend_t c3) {
    (void)c1;
    (void)c2;
    (void)c3;

    tile_common_init(c0);
}
#endif
