#ifndef __PACKET_HANDLE
#define __PACKET_HANDLE

#include "packet.h"
#include "global.h"
#include "led.h"
#include "buzzer.h"


/**
*@brief Serial port command callback processing
*@param frame dataframe
*@retval void
*/
void packet_led_handle(struct PacketRawFrame *frame);

/**
*@brief Serial port command callback processing
*@param frame dataframe
*@retval void
*/
void packet_buzzer_handle(struct PacketRawFrame *frame);

/**
*@brief Serial port command callback processing
*@param frame dataframe
*@retval void
*/

void packet_motor_handle(struct PacketRawFrame *frame);


void packet_handle_init(void) ;

#endif


