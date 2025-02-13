/**
 * @file buzzer.h
 * @author 
 * @brief Declaration of functions and data structures for hardware & system-independent buzzer control
 * @version 0.1
 * @date 2023-05-18
 *
 * @copyright Copyright (c) 2023
 *
 */

#ifndef __BUZZER_H_
#define __BUZZER_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum {
  BUZZER_STAGE_START_NEW_CYCLE,
  BUZZER_STAGE_WAITING_OFF,
  BUZZER_STAGE_WAITING_PERIOD_END,
  BUZZER_STAGE_IDLE,
} BuzzerStageEnum;


/**
  *@defgroup Buzzer Buzzer
  * @{
*/


/**
  * @brief Buzzer control parameter structure
  *
*/
typedef struct {
  uint16_t freq;      /**< @brief Buzzer frequency */
  uint32_t ticks_on;  /**< @brief Duration of buzzer sound in a cycle, in milliseconds */
  uint32_t ticks_off; /**< @brief Duration of buzzer silence in a cycle, in milliseconds */
  uint16_t repeat;    /**< @brief Number of times the buzzer "beeps" */
} BuzzerCtrlTypeDef;


/**
  * @brief Buzzer object structure
  *
*/
typedef struct BuzzerObject BuzzerObjectTypeDef;
struct BuzzerObject {
  uint32_t id;

  BuzzerStageEnum stage;   /**< @brief Current state of the buzzer control state machine */
  uint32_t ticks_count;    /**< @brief Millisecond counter, used to accumulate milliseconds to switch state machine states */
  BuzzerCtrlTypeDef ctrl_structure; /**< @brief Buzzer control parameters */

  // Porting interface
  /**
    * @brief Retrieve control block from the queue
    * @attention This function must be non-blocking
    * @param [out] p Pointer to the memory where the retrieved data is stored
    * @retval 0  Success
    * @retval !=0 Failure
  */
  int (*get_ctrl_block)(BuzzerObjectTypeDef *self, BuzzerCtrlTypeDef *p);
  /**
    * @brief Push a control block into the queue
    * @param p Pointer to the control block to be pushed
    * @retval 0  Success
    * @retval !=0 Failure
  */
  int (*put_ctrl_block)(BuzzerObjectTypeDef *self, BuzzerCtrlTypeDef *p);

  /**
    * @brief Set the inversion frequency of the corresponding IO port of the buzzer
    * @param freq New inversion frequency
    * @retval None.
   */
  void (*set_pwm)(BuzzerObjectTypeDef *self, uint32_t freq);
};

/**
  * @brief Initialize the memory space of a BuzzerObjectTypeDef instance with default data
  * @param self Pointer to the object to be initialized
  * @retval None.
*/
void buzzer_object_init(BuzzerObjectTypeDef *self);

/**
  * @brief Buzzer task handler function
  * @attention The user must call this function periodically to handle buzzer operations
  * @param self Pointer to the Buzzer object
  * @param period Interval in milliseconds between this call and the previous call
  * @retval None.
*/
void buzzer_task_handler(BuzzerObjectTypeDef *self, uint32_t period);

/**
  * @brief Turn the buzzer on continuously
  * @param self Pointer to the Buzzer object
  * @param freq Buzzer frequency
  * @retval 0 Success
  * @retval !=0 Failure
*/
int buzzer_on(BuzzerObjectTypeDef *self, uint32_t freq);

/**
  * @brief Turn the buzzer off
  * @param self Pointer to the Buzzer object
  * @retval 0 Success
  * @retval !=0 Failure
*/
int buzzer_off(BuzzerObjectTypeDef *self);

/**
  * @brief Make the buzzer beep with a fixed period and duty cycle
  * @param self Pointer to the Buzzer object
  * @param freq Buzzer frequency
  * @param ticks_on Duration of buzzer sound in a cycle, in milliseconds
  * @param ticks_off Duration of buzzer silence in a cycle, in milliseconds
  * @param repeat Number of times the buzzer beeps. If the given repeat count is 0, it will beep continuously
  * @retval 0 Success
  * @retval !=0 Failure
 */
int buzzer_didi(BuzzerObjectTypeDef *self, uint32_t freq, uint32_t ticks_on, uint32_t ticks_off, uint32_t repeat);

/** @} */
#endif
