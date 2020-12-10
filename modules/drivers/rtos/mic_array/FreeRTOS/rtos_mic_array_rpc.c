// Copyright (c) 2020, XMOS Ltd, All rights reserved

#include "drivers/rtos/rpc/api/rtos_rpc.h"

#include "drivers/rtos/mic_array/FreeRTOS/rtos_mic_array.h"

__attribute__((fptrgroup("rtos_mic_array_rx_fptr_grp")))
static size_t mic_array_remote_rx(
        rtos_mic_array_t *mic_array_ctx,
        int32_t sample_buf[][MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS],
        size_t sample_count,
        unsigned timeout)
{
    rtos_intertile_address_t *host_address = &mic_array_ctx->rpc_config->host_address;
    rtos_mic_array_t *host_ctx_ptr = mic_array_ctx->rpc_config->host_ctx_ptr;
    size_t ret;

    xassert(host_address->port >= 0);

    const rpc_param_desc_t rpc_param_desc[] = {
            RPC_PARAM_TYPE(mic_array_ctx),
            RPC_PARAM_OUT_BUFFER(sample_buf[0], sample_count * (MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS)),
            RPC_PARAM_TYPE(sample_count),
            RPC_PARAM_TYPE(timeout),
            RPC_PARAM_RETURN(size_t),
            RPC_PARAM_LIST_END
    };

    rpc_client_call_generic(
            host_address->intertile_ctx, host_address->port, 0, rpc_param_desc,
            &host_ctx_ptr, &sample_buf[0][0], &sample_count, &timeout, &ret);

    return ret;
}

static int mic_array_rx_rpc_host(rpc_msg_t *rpc_msg, uint8_t **resp_msg)
{
    int msg_length;

    rtos_mic_array_t *mic_array_ctx;
    int32_t (*sample_buf)[MIC_DUAL_NUM_CHANNELS + MIC_DUAL_NUM_REF_CHANNELS];
    size_t sample_count;
    unsigned timeout;
    size_t ret;

    /* Here, because the buffer is output only, the buffer
    pointer will not get set. */
    rpc_request_unmarshall(
            rpc_msg,
            &mic_array_ctx, &sample_buf, &sample_count, &timeout, &ret);

    /* Instead allocate a buffer with the length parameter
    before calling, as this is what would be done by
    the client. Alternatively this could be allocated
    statically or on the stack if the length is known
    at compile time. */
    sample_buf = pvPortMalloc(sample_count * sizeof(sample_buf[0]));

    ret = rtos_mic_array_rx(mic_array_ctx, sample_buf, sample_count, timeout);

    msg_length = rpc_response_marshall(
            resp_msg, rpc_msg,
            mic_array_ctx, sample_buf, sample_count, timeout, ret);

    /* The data from buffer has been copied into the response
    message. Since buffer was allocated on the heap, free
    it now. */
    vPortFree(sample_buf);

    return msg_length;
}

static void mic_array_rpc_thread(rtos_intertile_address_t *client_address)
{
    int msg_length;
    uint8_t *req_msg;
    uint8_t *resp_msg;
    rpc_msg_t rpc_msg;
    rtos_intertile_t *intertile_ctx = client_address->intertile_ctx;
    uint8_t intertile_port = client_address->port;

    for (;;) {
        /* receive RPC request message from client */
        msg_length = rtos_intertile_rx(intertile_ctx, intertile_port, (void **) &req_msg, portMAX_DELAY);

        rpc_request_parse(&rpc_msg, req_msg);

        /* IGNORING rpc_msg->fcode, assuming the port is only used by this mic context */

        msg_length = mic_array_rx_rpc_host(&rpc_msg, &resp_msg);

        vPortFree(req_msg);

        /* send RPC response message to client */
        rtos_intertile_tx(intertile_ctx, intertile_port, resp_msg, msg_length);
        vPortFree(resp_msg);
    }
}

__attribute__((fptrgroup("rtos_driver_rpc_host_start_fptr_grp")))
static void mic_array_rpc_start(
        rtos_driver_rpc_t *rpc_config)
{
    xassert(rpc_config->host_task_priority >= 0);

    for (int i = 0; i < rpc_config->remote_client_count; i++) {

        rtos_intertile_address_t *client_address = &rpc_config->client_address[i];

        xassert(client_address->port >= 0);

        xTaskCreate(
                    (TaskFunction_t) mic_array_rpc_thread,
                    "mic_array_rpc_thread",
                    RTOS_THREAD_STACK_SIZE(mic_array_rpc_thread),
                    client_address,
                    rpc_config->host_task_priority,
                    NULL);
    }
}

void rtos_mic_array_rpc_config(
        rtos_mic_array_t *mic_array_ctx,
        unsigned intertile_port,
        unsigned host_task_priority)
{
    rtos_driver_rpc_t *rpc_config = mic_array_ctx->rpc_config;

    if (rpc_config->remote_client_count == 0) {
        /* This is a client */
        rpc_config->host_address.port = intertile_port;
    } else {
        for (int i = 0; i < rpc_config->remote_client_count; i++) {
            rpc_config->client_address[i].port = intertile_port;
        }
        rpc_config->host_task_priority = host_task_priority;
    }
}

void rtos_mic_array_rpc_client_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *host_intertile_ctx)
{
    mic_array_ctx->rpc_config = rpc_config;
    mic_array_ctx->rx = mic_array_remote_rx;
    rpc_config->rpc_host_start = NULL;
    rpc_config->remote_client_count = 0;
    rpc_config->host_task_priority = -1;

    /* This must be configured later with rtos_mic_array_rpc_config() */
    rpc_config->host_address.port = -1;

    rpc_config->host_address.intertile_ctx = host_intertile_ctx;
    rpc_config->host_ctx_ptr = (void *) s_chan_in_word(host_intertile_ctx->c);
}

void rtos_mic_array_rpc_host_init(
        rtos_mic_array_t *mic_array_ctx,
        rtos_driver_rpc_t *rpc_config,
        rtos_intertile_t *client_intertile_ctx[],
        size_t remote_client_count)
{
    mic_array_ctx->rpc_config = rpc_config;
    rpc_config->rpc_host_start = mic_array_rpc_start;
    rpc_config->remote_client_count = remote_client_count;

    /* This must be configured later with rtos_mic_array_rpc_config() */
    rpc_config->host_task_priority = -1;

    for (int i = 0; i < remote_client_count; i++) {
        rpc_config->client_address[i].intertile_ctx = client_intertile_ctx[i];
        s_chan_out_word(client_intertile_ctx[i]->c, (uint32_t) mic_array_ctx);

        /* This must be configured later with rtos_mic_array_rpc_config() */
        rpc_config->client_address[i].port = -1;
    }
}
