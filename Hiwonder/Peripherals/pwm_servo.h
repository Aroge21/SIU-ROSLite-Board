/**
 *@file pwm_servo.h
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief st7735 display driver
 *@version 0.1
 *@date 2023-05-05
 *
 *@copyright Copyright (c) 2023
 *
 */

#ifndef __PWM_SERVO_H_
#define __PWM_SERVO_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct PWMServoObject  PWMServoObjectTypeDef;

/**
 *@brief PWM servo object structure
 */
struct PWMServoObject {
    int id;/**< @brief servo ID */
    int offset;/**< @brief servo deviation */
    int target_duty;/**< @brief target pulse width */
    int current_duty;/**< @brief Current pulse width */
    int duty_raw;/**< @brief The pulse width of the machine, the pulse width of the final write to the timer, contains offset, == current_duty + offset */

/*Variables required for speed control */
    uint32_t duration;/**< @brief The time when the servo moves from the current angle to the specified angle, that is, the control speed Unit: ms */
    float duty_inc;/**< @brief Pulse width increment for each position update */
    int  inc_times;/**< @brief The number of times that need to be incremented */
    bool is_running;/**< @brief Is the servo during rotation */
    bool duty_changed;/**< @brief Whether the pulse width has been changed */

/*Externally provided, hardware abstract interface */
    void (*write_pin)(uint32_t new_state);/*IO port level settings */
};

/**
 *@brief servo object initialization
 *@param object To initialize the servo object pointer
 *@retval None.
*/
void pwm_servo_object_init(PWMServoObjectTypeDef *object);

/**
 *@brief servo pulse width control
 *@detials calculates the pulse width required for the current speed and implements control, which requires 50ms to be called once.
 *@param self The servo object pointer that needs to be controlled
 *@retval None.
*/
void pwm_servo_duty_compare(PWMServoObjectTypeDef *self);

/**
 *@brief Set the servo angle
 *@detials is used to set the angle of the servo, but it is actually only set a few internal variables of the servo object. The servo will not be actually controlled immediately. The actual servo is calculated and controlled by pwm_servo_duty_compare.
 *@param self The servo object pointer that needs to be controlled
 *@param The new pulse width of the servo is an integer value between 500 and 2500
 *@param None.
*/
void pwm_servo_set_position (PWMServoObjectTypeDef *self, uint32_t duty, uint32_t duration);


/**
 *@brief Set servo deviation
 *@param self The servo object pointer that needs to be controlled
 *@param New servo deviation
 *@param None.
*/
void pwm_servo_set_offset(PWMServoObjectTypeDef *self, int offset);

#endif

