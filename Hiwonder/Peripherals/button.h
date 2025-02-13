/**
 *@file button.h
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief What are the related programs and number structures for hardware-independent key detection?
 *@version 0.1
 *@date 2023-05-08
 *
 *@copyright Copyright (c) 2023
 *
 */
#ifndef __BUTTON_H_
#define __BUTTON_H_

#include <stdio.h>
#include <stdint.h>

typedef enum {
    BUTTON_STAGE_NORMAL,
    BUTTON_STAGE_PRESS,
    BUTTON_STAGE_LONGPRESS,
} ButtonStageEnum;

/**
  *@defgroup Button Button
  *@{
*/

/**
  *@brief button event enumeration
  *
*/
typedef enum {
    BUTTON_EVENT_PRESSED = 0x01,/**< @brief button is pressed */
    BUTTON_EVENT_LONGPRESS = 0x02,/**< @brief button is long pressed */
    BUTTON_EVENT_LONGPRESS_REPEAT = 0x04,/**< @brief button is long pressed and triggered */
    BUTTON_EVENT_RELEASE_FROM_LP = 0x08,/**< @brief button release from long press */
    BUTTON_EVENT_RELEASE_FROM_SP = 0x10,/**< @brief button release from short press */
    BUTTON_EVENT_CLICK = 0x20,/**< @brief button is clicked */
    BUTTON_EVENT_DOUBLE_CLICK = 0x40,/**< @brief button is double-clicked */
    BUTTON_EVENT_TRIPLE_CLICK = 0x80,/**< @brief button is hit by three combo */
} ButtonEventIDEnum;



typedef struct ButtonObject ButtonObjectTypeDef;

/**
  *@brief button event object structure
  *
*/
typedef struct {
    ButtonObjectTypeDef *button;/**< @brief button object pointer that triggers the event */
    ButtonEventIDEnum event;/**< @brief button event ID */
} ButtonEventObjectTypeDef;

/**
  *@brief button event callback function type
  *
*/
typedef void (*ButtonEventCallbackFuncTypeDef)(ButtonObjectTypeDef *self,  ButtonEventIDEnum event);

/**
  *@brief button object structure
  *
*/
struct ButtonObject {
	uint32_t id;

    ButtonStageEnum stage;/**< @brief key scans the current state of the state machine */
    uint32_t last_pin_raw;/**< @brief The original IO port status read last time */
	uint32_t last_pin_filtered;/**< @brief The status of the IO port that passed the last time after deshaking */
    uint32_t combin_counter;/**< @brief combo count */
	uint32_t ticks_count;/**< @brief millisecond timing variable */

/*config */
    uint32_t combin_th;/**< @brief double-click maximum interval milliseconds*/
    uint32_t lp_th;/**< @brief Press the threshold milliseconds */
    uint32_t repeat_th;/**< @brief Long press and heavy trigger interval milliseconds */
    ButtonEventCallbackFuncTypeDef event_callback;/**< @brief event callback function pointer */

/*Migrate interface */
/**
      *@brief Read button IO status
      *@attention When the user implements this interface, its return value must be 1 when pressed and 0 when released.
      *@retval 0 Button not pressed
      *@retval 1 button has been pressed
    */
    uint32_t (*read_pin)(ButtonObjectTypeDef *self);

};


/**
  *@brief Initializes the memory space of a ButtonObjectTypeDef instance with default data
  *@param self object pointer to initialize
  *@retval None.
*/
void button_object_init(ButtonObjectTypeDef *self);

/**
  *@brief button task processing function
  *@attention The user must call this function regularly to scan
  *@param self button object pointer
  *@param period The number of milliseconds between this call and the last call
  *@retval None.
*/
void button_task_handler(ButtonObjectTypeDef *self, uint32_t period);

/**
  *@brief button event callback registration
  *@param self button object pointer
  *@param callback callback function pointer
  *@retval None.
*/
void button_register_callback(ButtonObjectTypeDef *self, ButtonEventCallbackFuncTypeDef callback);


/**
  *@brief button's default event callback
  *@param self The button object that triggers the callback
  *@param event The event that triggers this callback
  */
void button_defalut_event_callback(ButtonObjectTypeDef *self,  ButtonEventIDEnum event);

/**@} */
#endif

