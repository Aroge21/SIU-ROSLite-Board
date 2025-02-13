/**
 *@file led.h
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief The function of the hardware-independent LED light flash control is the data structure declaration
 *@version 0.1
 *@date 2023-05-13
 *
 *@copyright Copyright (c) 2023
 *
 */


#ifndef __LED_H_
#define __LED_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


#define LED_NUM  3

typedef enum {
    LED_STAGE_START_NEW_CYCLE,
    LED_STAGE_WATTING_OFF,
    LED_STAGE_WATTING_PERIOD_END,
    LED_STAGE_IDLE,
} LEDStageEnum;



/**
  *@defgroup LED LED
  *@{
*/


/**
  *@brief LED controlled parameter structure
  *
*/
typedef struct {
    uint32_t ticks_on;/**< @brief The duration of LED light up during the period, milliseconds */
    uint32_t ticks_off;/**< @brief The duration of LED off during the cycle, milliseconds */
    uint16_t repeat;/**< @brief LED flashing repeats */
} LEDCtrlTypeDef;


/**
  *@brief LED light object structure
  *
*/
typedef struct LEDObjectObject LEDObjectTypeDef;
struct LEDObjectObject {
	uint32_t id;
	
    LEDStageEnum stage;/**< @brief The current state of the LED control state machine */
    uint32_t ticks_count;/**< @brief millisecond count, used to switch state machine state by cumulative milliseconds */
    LEDCtrlTypeDef ctrl_structure;/**< @brief LED control parameters */

    //Migrate interface
    /**
      *@brief remove control block from queue
      *@attention This function must be non-blocking
      *@param [out] p Memory pointer to store the data withdrawal
      *@retval 0 Success
      *@retval !=0 Failed
    */
    int (*get_ctrl_block)(LEDObjectTypeDef *self, LEDCtrlTypeDef *p);
/**
      *@brief pushes a control block into the queue
      *@param p Pointer to the control block to be pushed
      *@retval 0 Success
      *@retval !=0 Failed
    */
    int (*put_ctrl_block)(LEDObjectTypeDef *self, LEDCtrlTypeDef *p);

/**
      *@brief Setting LED lights on and off
      *@param new_state LED New state @attention No matter whether the actual circuit is on high or low, this function must be turned on when writing 1 and off when writing 0
      *@retval None.
     */
    void (*set_pin)(LEDObjectTypeDef *self, uint32_t new_state);
};



/**
  *@brief Initializes the memory space of an LEDObjectTypeDef instance with default data
  *@param self object pointer to initialize
  *@retval None.
*/
void led_object_init(LEDObjectTypeDef *self);

/**
  *@brief LED task processing function
  *@attention The user must call this function regularly to handle the operation of the LED
  *@param self LED object pointer
  *@param period The number of milliseconds between this call and the last call
  *@retval None.
*/
void led_task_handler(LEDObjectTypeDef *self, uint32_t period);

/**
  *@brief Make the LED bright
  *@param self LED object pointer
  *@retval 0 Success
  *@retval !=0 Failed
*/
int led_on(LEDObjectTypeDef *self);

/**
  *@brief turns the LED off
  *@param self LED object pointer
  *@retval 0 Success
  *@retval !=0 Failed
*/
int led_off(LEDObjectTypeDef *self);


/**
  *@brief makes the LED flash at fixed cycle and duty cycle
  *@param self LED object pointer
  *@param ticks_on The duration of LED light up in the cycle, milliseconds
  *@param ticks_off The duration of LED off during the period, milliseconds
  *@param repeat Number of flashing repetitions, and it will keep flashing when the given number of repetitions is 0
  *@retval 0 Success
  *@retval !=0 Failed
 */
int led_flash(LEDObjectTypeDef *self, uint32_t ticks_on, uint32_t ticks_off, uint32_t repeat);

/**@} */
#endif

