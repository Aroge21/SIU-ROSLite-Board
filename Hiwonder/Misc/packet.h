/**
*@file protocol.h
*@brief The communication protocol processing with the host computer
*@author LuYongPing
*@date 20 MAY 2023
*/

#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#include <stdint.h>
#include <cmsis_os2.h>
#include "lwrb.h"
#include "lwmem.h"

#define PROTO_CONST_STARTBYTE1 0xAAu
#define PROTO_CONST_STARTBYTE2 0x55u
#define PACKET_PARSE_BUFFER_SIZE 64

#pragma pack(1)
struct PacketRawFrame {
    uint8_t start_byte1;
    uint8_t start_byte2;
    uint8_t function;
    uint8_t data_length;
    uint8_t data_and_checksum[257];
};
#pragma pack()

/**
 *@brief parser state machine state enumeration
 *
 */
enum PacketControllerState {
    PACKET_CONTROLLER_STATE_STARTBYTE1,/**< Looking for frame header tag 1 */
    PACKET_CONTROLLER_STATE_STARTBYTE2,/**< Looking for frame header tag 2 */
    PACKET_CONTROLLER_STATE_FUNCTION,/**< Frame function number is being processed */
    PACKET_CONTROLLER_STATE_LENGTH,/**< Frame length is being processed */
    PACKET_CONTROLLER_STATE_DATA,/**< Frame data is being processed */
    PACKET_CONTROLLER_STATE_CHECKSUM,/**Data verification is being processed */
};

/**
 *@brief frame function number enumeration
 *
 */
enum PACKET_FUNCTION {
    PACKET_FUNC_SYS = 0,
    PACKET_FUNC_LED,
    PACKET_FUNC_BUZZER,
    PACKET_FUNC_MOTOR,
    PACKET_FUNC_PWM_SERVO,
    PACKET_FUNC_BUS_SERVO,
    PACKET_FUNC_KEY,
    PACKET_FUNC_IMU,
    PACKET_FUNC_GAMEPAD,
    PACKET_FUNC_SBUS,
	PACKET_FUNC_OLED,
    
    PACKET_FUNC_RGB,
    
    PACKET_FUNC_NONE,
};

typedef void(*packet_handle)(struct PacketRawFrame *);

/**
 *@brief protocol parser
 *@details protocol parser, store the working state of the parser, state machine state, etc.
 */
struct PacketController {
    enum PacketControllerState state;/**< Current status of the parser state machine */
    struct PacketRawFrame frame;/**< Frames being processed by the parser */
    packet_handle handles[PACKET_FUNC_NONE];/**< parse operation list */
    int data_index;

    uint8_t *rx_dma_buffers[2];/**< DMA cache list */
    size_t rx_dma_buffer_size;/**< Size of a single DMA cache */
    volatile int rx_dma_buffer_index;/**< DMA cache index number currently being received */

    uint8_t *rx_fifo_buffer;/**< Receive FIFO cache */
    lwrb_t *rx_fifo;/**< Receive cached FIFO object */

    int (*send_packet)(struct PacketController *self, struct PacketRawFrame *frame);
    struct PacketRawFrame* tx_dma_buffer;/**< DMA cache being sent*/
};


/**
 *@brief Serial command callback registration
 *@param self protocol instance
 *@param func function ID
 *@param handle callback function
 *@retval None
*/
void packet_register_callback(struct PacketController *self, enum PACKET_FUNCTION func, packet_handle p);


/**
 *@brief Serial port protocol reception processing
 *@details Pass in data, switch state machine status according to state machine status and data, and complete the reception and analysis of the protocol
 *The parsed data frame will be pushed into the receiving frame queue
 *@param self protocol instance
 *@retval None
 */
void packet_recv(struct PacketController *self);

/**
 *@brief Serial port protocol sending processing
 *
 *@param self protocol instance
 *@param func function number
 *@param data segment
 *@param data_len Data segment length
 *@retval ==0 Success
 *@retval !=0 Failed
 */
int packet_transmit(struct PacketController *self, uint8_t func, void* data, size_t data_len);

#endif
