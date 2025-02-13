/**
 *@file led_portting.c
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief onboard LED light control example and interface implementation
 *@version 0.1
 *@date 2023-05-23
 *
 *@copyright Copyright (c) 2023
 *
 */

#include "led.h"
#include "gpio.h"
#include "global_conf.h"
#include "packet.h"
#include "lwmem_porting.h"

/*System-wide global variables */
LEDObjectTypeDef *leds[LED_NUM];
static osMessageQueueId_t led_ctrl_ququeHandle[LED_NUM];/*LED1 control queue Handle */

static void led_set_pin(LEDObjectTypeDef *self, uint32_t level);/*LED_SYS LED write IO port interface */
static int put_ctrl_block(LEDObjectTypeDef *self, LEDCtrlTypeDef *p);/*Control the enqueue interface */
static int get_ctrl_block(LEDObjectTypeDef *self, LEDCtrlTypeDef *p);/*Control the dequeue interface */

/**
  *@brief Initialize LED-related memory and variables
  *@retval None.
*/
void leds_init(void)
{
/*Create LED1 control queue */
	const osMessageQueueAttr_t led_ctrl_quque_attributes[LED_NUM] = {{ .name = "led1_ctrl_quque" },
                                                                      { .name = "led2_ctrl_quque" },
                                                                      { .name = "led3_ctrl_quque" }};

    for(int i = 0 ; i < LED_NUM ; i++){
        led_ctrl_ququeHandle[i] = osMessageQueueNew(5, sizeof(LEDCtrlTypeDef), &led_ctrl_quque_attributes[i]);
        leds[i] = LWMEM_CCM_MALLOC(sizeof(LEDObjectTypeDef)); 
        led_object_init(leds[i]);
        leds[i]->id = i+1;
        leds[i]->set_pin = led_set_pin;
        leds[i]->get_ctrl_block = get_ctrl_block;
        leds[i]->put_ctrl_block = put_ctrl_block;
    }

   //packet_register_callback(&packet_controller, PACKET_FUNC_LED, packet_handler);
}

/**
  *@brief timer callback
  *@detials Refresh LED status regularly
  *@retval None.
  *
*/
void led_timer_callback(void *argument) {
    for(int i = 0 ; i < LED_NUM ; i++)
    {
        led_task_handler(leds[i], LED_TASK_PERIOD);
    }
}


static void led_set_pin(LEDObjectTypeDef *self, uint32_t level)
{
    if(self->id == 1){
        HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, (GPIO_PinState)(level ^ 1u));
    }else if(self->id == 2){
        HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, (GPIO_PinState)(level ^ 1u));
    }else{
        HAL_GPIO_WritePin(LED3_GPIO_Port, LED3_Pin, (GPIO_PinState)(level ^ 0u));
    }
}


/**
  *@brief LED control queued interface
  *@param [in] p Pointer for the control parameter structure to be dequeued
  *@retval 0 Success
  *@retval !=0 Failed
  *
*/
static int put_ctrl_block(LEDObjectTypeDef *self, LEDCtrlTypeDef *p) {
    if(self->id == 1){
        return (int)osMessageQueuePut(led_ctrl_ququeHandle[0], p, 0, 0);
    }else if(self->id == 2){
        return (int)osMessageQueuePut(led_ctrl_ququeHandle[1], p, 0, 0);
    }else{
        return (int)osMessageQueuePut(led_ctrl_ququeHandle[2], p, 0, 0);
    }
}

/**
  *@brief LED control queue queuing interface
  *@param [out] p Storage pointer for dequeuing data
  *@retval 0 Success
  *@retval !=0 Failed
  *
*/
static int get_ctrl_block(LEDObjectTypeDef *self, LEDCtrlTypeDef *p) {
    if(self->id == 1){
        return (int)osMessageQueueGet(led_ctrl_ququeHandle[0], p, 0, 0);
    }else if(self->id == 2){
        return (int)osMessageQueueGet(led_ctrl_ququeHandle[1], p, 0, 0);
    }else{
        return (int)osMessageQueueGet(led_ctrl_ququeHandle[2], p, 0, 0);
    }
}

