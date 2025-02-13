/**
 *@file lwmem_porting.c
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief Hardware-related processing received by SBUS protocol
 *@version 0.1
 *@date 2023-05-18
 *
 *@copyright Copyright (c) 2023
 *
 */

#include <stdio.h>
#include "SBus.h"
#include "lwmem_porting.h"
#include "lwrb.h"
#include "usart.h"
#include "global.h"
#include "packet_reports.h"

#define SBUS_RX_DMA_BUFFER_SIZE 32/*SBUS DMA Receive cache length */
#define SBUS_RX_FIFO_BUFFER_SIZE 512/*SBUS FIFO cache length */

/*SBUS1 related variables */
static uint8_t *subs1_rx_dma_buffers[2];
static uint32_t sbus1_rx_dma_buffer_index;
static uint8_t *sbus1_rx_fifo_buffer;
static lwrb_t *sbus1_rx_fifo;
static void sbus_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length);

/*SBUS External Exposed Variables */
SBusStatusObjectTypeDef *sbus1_status;

/*External variables required by SBUS1 */
extern osSemaphoreId_t sbus_data_readyHandle;/*subs data ready semaphore Handle */
extern osSemaphoreId_t sbus_data_ready_01_Handle;
extern osEventFlagsId_t sbus_data_ready_event_Handle;


/**
 *@brief SBUS initialization
 *Initialize SBUS-related memory and variables and start SBUS reception
 *@retval None.
 */
void sbus_init(void)
{
    sbus1_status = LWMEM_CCM_MALLOC(sizeof(SBusStatusObjectTypeDef));
    sbus1_status->type_id = OBJECT_TYPE_ID_GAMEPAD_STATUS;

/*DMA Receive cache initialization */
    subs1_rx_dma_buffers[0] = LWMEM_RAM_MALLOC(SBUS_RX_DMA_BUFFER_SIZE);/*DMA cache cannot be placed on CCMRAM */
    subs1_rx_dma_buffers[1] = LWMEM_RAM_MALLOC(SBUS_RX_DMA_BUFFER_SIZE);
    sbus1_rx_dma_buffer_index = 0;

/*Receive FIFO initialization */
    sbus1_rx_fifo_buffer  = LWMEM_CCM_MALLOC(SBUS_RX_FIFO_BUFFER_SIZE);
    sbus1_rx_fifo = LWMEM_CCM_MALLOC(sizeof(lwrb_t));
    lwrb_init(sbus1_rx_fifo, sbus1_rx_fifo_buffer, SBUS_RX_FIFO_BUFFER_SIZE);

}


/**
 *@brief SBUS receive event callback
 *This function triggers the DMA reception cache to press the received data into the receiving FIFO cache after the DMA reception cache is full or when it is idle, and then the reception task completes the analysis and processing.
 *Because the minimum sending interval of SBUS protocol is 4ms, and the DMA cache size is set to 32, the idle event is always triggered without triggering the DMA full event.
 *@attention It should be noted that not all remote control receivers will complete one frame at a time, so one frame of data may trigger multiple idle events, and the idle event cannot be used directly for frame segmentation.
 *Because the reception start position is not necessarily the frame header position, it is also impossible to use the DMA reception length directly for segmentation.
 *If you must use DMA length/idle events for frame segmentation, you must first complete frame synchronization, that is, receive a complete and correct frame and then start DMA reception. Ensure that the first byte received by DMA is the frame header.
 *In this implementation, all received data are pooled into a byte stream, and then divided by fixed values ​​at the head and end of the frame.
 *@param huart serial port example
 *@param Pos Number of bytes received
 *@retval None.
 */
static void sbus_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length)
{
    uint32_t cur_index = sbus1_rx_dma_buffer_index;/*Get the index number of the current DMA cache */
    sbus1_rx_dma_buffer_index ^= 1;
    if(length < SBUS_RX_DMA_BUFFER_SIZE) {
        HAL_UART_AbortReceive(&huart5);
    }
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, (uint8_t*)subs1_rx_dma_buffers[sbus1_rx_dma_buffer_index], SBUS_RX_DMA_BUFFER_SIZE);
    lwrb_write(sbus1_rx_fifo, subs1_rx_dma_buffers[cur_index], length);/*Write the received data to fifo ring */
    if(lwrb_get_full(sbus1_rx_fifo) >= 25) {/*SBUS The data size per frame is 25 bytes, and the analysis will be performed once every time it exceeds 25 bytes */
        osSemaphoreRelease(sbus_data_ready_01_Handle);/*Assert SBUS data ready semaphore */
    }
}


/**
 *@brief SBUS receiving task entry function
 *@param argument entry parameters
 *@retval None.
 */
#if ENABLE_SBUS
void sbus_rx_task_entry(void *argument)
{
    extern osMessageQueueId_t lvgl_event_queueHandle;
    extern SBusStatusObjectTypeDef sbus_status_disp;

    static uint8_t buf_temp[32];

    sbus_init();

/*Start receiving */
start_sbus_receive:
    HAL_UART_AbortReceive(&huart5);
    HAL_UART_RegisterRxEventCallback(&huart5, sbus_dma_receive_event_callback);/*Register to receive event callback */
/*Use ReceiveToIdle_DMA for reception, which interrupts when the DMA cache is full or interrupts when the reception is idle and triggers the receive event callback */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart5, (uint8_t*)subs1_rx_dma_buffers[sbus1_rx_dma_buffer_index], SBUS_RX_DMA_BUFFER_SIZE);
/*parse loop */
    for(;;) {
        if(osSemaphoreAcquire(sbus_data_ready_01_Handle, 500) != osOK) {/*Wait for data to be ready */
            goto start_sbus_receive;
        }
        for(;;) {
            memset(buf_temp, 0, 25);
            if(lwrb_peek(sbus1_rx_fifo, 0, buf_temp, 25) == 25) {/*Get 25 bytes from cache */
                if(sbus_decode_frame(buf_temp, sbus1_status) == 0) {/*Try to parse until the bytes are not enough */
                    lwrb_skip(sbus1_rx_fifo, 25);
                    PacketReportSBusTypeDef report;
                    for(int i = 0; i < 16; ++i) {
                        report.channels[i] = sbus1_status->channels[i];
                    }
                    report.ch17 = sbus1_status->ch17;
                    report.ch18 = sbus1_status->ch18;
                    report.signal_loss = sbus1_status->signal_loss;
                    report.fail_safe = sbus1_status->fail_safe;
                    packet_transmit(&packet_controller, PACKET_FUNC_SBUS, &report, sizeof(PacketReportSBusTypeDef));
                } else {
                    lwrb_skip(sbus1_rx_fifo, 1);
                }
            } else {
                break;
            }
        }
    }
}
#endif

