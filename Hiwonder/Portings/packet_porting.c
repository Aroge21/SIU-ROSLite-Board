/**
 *@file packet_portting.c
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief serial port protocol interface implementation
 *@version 0.1
 *@date 2023-05-23
 *
 *@copyright Copyright (c) 2023
 *
 */

#include "global.h"
#include "lwrb.h"
#include "usart.h"
#include "packet.h"
#include "cmsis_os2.h"
#include <stdio.h>
#include "lwmem_porting.h"
#include "packet_handle.h"

#define PACKET_RX_FIFO_BUFFER_SIZE 2048/*FIFO cache length */
#define PACKET_RX_DMA_BUFFER_SIZE 256/*Single DMA cache length */

/*Variables exposed to the outside */
struct PacketController packet_controller;/*Protocol controller instance */

/*Internal function */
static void packet_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length);
static void packet_dma_transmit_finished(UART_HandleTypeDef *huart);
static int send_packet(struct PacketController *self, struct PacketRawFrame *frame);
static void packet_uart_error_callblack(UART_HandleTypeDef *huart);

/*External variables that serial port control depends on */
extern osSemaphoreId_t packet_tx_idleHandle;
extern osSemaphoreId_t packet_rx_not_emptyHandle;
extern osMessageQueueId_t packet_tx_queueHandle;

/**
 *@brief Initialize packet controller object
 *@retval void
 */
void packet_init(void)
{
    memset(&packet_controller, 0, sizeof(packet_controller));
    packet_controller.state = PACKET_CONTROLLER_STATE_STARTBYTE1;
    packet_controller.data_index = 0;

/*DMA Receive cache initialization */
    static uint8_t rx_dma_buffer1[PACKET_RX_DMA_BUFFER_SIZE];
    static uint8_t rx_dma_buffer2[PACKET_RX_DMA_BUFFER_SIZE];

    packet_controller.rx_dma_buffers[0] = rx_dma_buffer1;
    packet_controller.rx_dma_buffers[1] = rx_dma_buffer2;
    packet_controller.rx_dma_buffer_size = PACKET_RX_DMA_BUFFER_SIZE;
    packet_controller.rx_dma_buffer_index = 0;

/*Receive FIFO initialization */
    packet_controller.rx_fifo_buffer  = LWMEM_CCM_MALLOC(PACKET_RX_FIFO_BUFFER_SIZE);
    packet_controller.rx_fifo = LWMEM_CCM_MALLOC(sizeof(lwrb_t));
    lwrb_init(packet_controller.rx_fifo, packet_controller.rx_fifo_buffer, PACKET_RX_FIFO_BUFFER_SIZE);

/*Send interface */
    packet_controller.send_packet = send_packet;
}

/**
 *@brief sending interface
 *@details In fact, it will not send directly, but will press the frame pointer to be sent into the send queue, waiting for the sending task operation hardware to complete the sending
 *
 *@param self protocol controller instance
 *@param frame The frame to be sent
 *@return int
 */
static int send_packet(struct PacketController *self, struct PacketRawFrame *frame)
{
    return osMessageQueuePut(packet_tx_queueHandle, &frame, 0, 10);
}

/**
 *@brief Serial port protocol packet receiving event callback
 *This function triggers the DMA reception cache to press the received data into the receiving FIFO cache after the DMA reception cache is full or when it is idle, and then the reception task completes the analysis and processing.
 *@param huart serial port example
 *@param Pos Number of bytes received
 *@retval void
 */
static void packet_dma_receive_event_callback(UART_HandleTypeDef *huart, uint16_t length)
{
    int cur_index = packet_controller.rx_dma_buffer_index;/*Get the index number of the current DMA cache */
    packet_controller.rx_dma_buffer_index ^= 1;
    HAL_UART_AbortReceive(&huart1);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, packet_controller.rx_dma_buffers[packet_controller.rx_dma_buffer_index], PACKET_RX_DMA_BUFFER_SIZE);
    lwrb_write(packet_controller.rx_fifo, packet_controller.rx_dma_buffers[cur_index], length);/*Write the received data to fifo ring */
    osSemaphoreRelease(packet_rx_not_emptyHandle);/*Assert the cached non-empty signal */
}


/**
 *@brief Start serial port protocol reception
 *@retval void
 */
void packet_start_recv(void)
{
    HAL_UART_AbortReceive(&huart1);
    HAL_UART_RegisterCallback(&huart1, HAL_UART_ERROR_CB_ID, packet_uart_error_callblack);
    HAL_UART_RegisterRxEventCallback(&huart1, packet_dma_receive_event_callback);/*Register to receive event callback */
/*Use ReceiveToIdle_DMA for reception, which interrupts when the DMA cache is full or interrupts when the reception is idle and triggers the receive event callback */
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, packet_controller.rx_dma_buffers[packet_controller.rx_dma_buffer_index], PACKET_RX_DMA_BUFFER_SIZE);
/*Start receiving */
}

/**
 *@brief serial port error handling
*Currently, the actual test is that ModemManager will occupy the newly inserted serial port (/dev/ttyACM*device will be mistaken for Modem) and cause an error.
 *We need to start receiving again after an error
 *@param huart serial port example
 *@retval None.
 *
*/
static void packet_uart_error_callblack(UART_HandleTypeDef *huart)
{
    packet_start_recv();
}
/**
 *@brief The receiving task portal of the serial port protocol package
 *Wait for the serial port to cache non-empty semaphores, and then retrieve data from the serial port to parse and process it
 *The serial port cache non-empty semaphore is set by the serial port receiving event
 *@param argument Reserved
 *@retval void
 */
void packet_rx_task_entry(void *argument)
{
    osSemaphoreAcquire(packet_rx_not_emptyHandle, 0);/*The default signal is not zero, clear it first */
    __HAL_UNLOCK(&huart1);
    packet_start_recv();
    for(;;) {
        osSemaphoreAcquire(packet_rx_not_emptyHandle, osWaitForever);/*Waiting for receiving cache is not empty */
        packet_recv(&packet_controller);
    }
}


/**
 *@brief The sending task portal of the serial port protocol packet
 *When the serial port sends it completes, check whether the queue is empty. If it is not empty, data will be directly retrieved from the queue and completed a sending.
 *The sending idle flag is set to end the interrupt until the queue is empty
 *https://note.youdao.com/s/D1lHKSH0
 *@param argument Reserved
 *@retval void
 */
void packet_tx_task_entry(void *argument)
{
    for(;;) {
        osSemaphoreAcquire(packet_tx_idleHandle, osWaitForever);/*Wait for the idle signal to be sent */
        osStatus_t status = osMessageQueueGet(packet_tx_queueHandle, &packet_controller.tx_dma_buffer, NULL, osWaitForever);/*Extract data from the send queue */
        if(osOK == status) {
            HAL_UART_RegisterCallback(&huart1, HAL_UART_TX_COMPLETE_CB_ID, packet_dma_transmit_finished);/*Register DMA Receive Complete Callback */
            HAL_UART_Transmit_DMA(&huart1, (uint8_t*)packet_controller.tx_dma_buffer, packet_controller.tx_dma_buffer->data_length + 5);/*Trigger DMA send*/
        }
    }
}

/**
 *@brief Serial port protocol package DMA send complete callback
 *@param huart
 *@retval void
 */

static void packet_dma_transmit_finished(UART_HandleTypeDef * huart)
{
    lwmem_free(packet_controller.tx_dma_buffer);
    osStatus_t status = osMessageQueueGet(packet_tx_queueHandle, &packet_controller.tx_dma_buffer, NULL, 0);/*Extract data from the send queue */
    if(osOK == status) {
        HAL_UART_RegisterCallback(&huart1, HAL_UART_TX_COMPLETE_CB_ID, packet_dma_transmit_finished);/*Register DMA Receive Complete Callback */
        HAL_UART_Transmit_DMA(&huart1, (uint8_t*)packet_controller.tx_dma_buffer, packet_controller.tx_dma_buffer->data_length + 5);/*Trigger DMA send*/
    } else {
        osSemaphoreRelease(packet_tx_idleHandle);/*Set to send an idle signal */
    }
}

