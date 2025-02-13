/**
 *@file pwm_servo.c
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief PWM servo driver hardware-independent code
 *@version 0.1
 *@date 2023-05-07
 *
 *@copyright Copyright (c) 2023
 *
 */

#include "pwm_servo.h"
#include <string.h>

void pwm_servo_duty_compare(PWMServoObjectTypeDef *self)//Pulse width change comparison and speed control
{
    //Recalculate the servo control parameters according to the newly set target
    if(self->duty_changed) {
        self->duty_changed = false;
        self->inc_times = self->duration / 20;//Calculate the number of times that need to be incremented
        if(self->target_duty > self->current_duty) {/*Calculate the total position transformation amount */
            self->duty_inc = (float)(-(self->target_duty - self->current_duty));
        } else {
            self->duty_inc = (float)(self->current_duty - self->target_duty);
        }
        self->duty_inc /= (float)self->inc_times;/*Calculate the position increment per control cycle */
        self->is_running = true;//The servo starts to operate
    }
		
		//The steering wheel needs to be controlled to reach a new position
    if(self->is_running) {
        --self->inc_times;
        if(self->inc_times == 0) {
            self->current_duty = self->target_duty;//The last increment is to assign the set value to the current value to ensure that the maximum position is correct
            self->is_running = false;//After the set position, the servo stops moving
        } else {
            self->current_duty = self->target_duty + (int)(self->duty_inc * self->inc_times);
        }
    }
    self->duty_raw = self->current_duty + self->offset;//The servo deviation must be added to the action
}

void pwm_servo_set_position (PWMServoObjectTypeDef *self, uint32_t duty, uint32_t duration)
{
    duration = duration < 20 ? 20 : (duration > 30000 ? 30000 : duration);/*Limit minimum/longest exercise time */
	duty = duty > 2500 ? 2500 : (duty < 500 ? 500 : duty);/*Restricted pulse width maximum/minimum value */
    self->target_duty = duty;
    self->duration = duration;
    self->duty_changed = true;/*Mark the target position and send the transformation, let pwm_servo_duty_compare calculate the new motion parameters */
}

void pwm_servo_set_offset(PWMServoObjectTypeDef *self, int offset)
{
    offset = offset < -100 ? -100 : (offset > 100 ? 100 : offset);/*Limit minimum/maximum deviation, different servo limits will be different, but 100 is a good choice*/
    self->offset = offset;
}

void pwm_servo_object_init(PWMServoObjectTypeDef *obj)
{
    memset(obj, 0, sizeof(PWMServoObjectTypeDef));
    obj->current_duty = 1500;/*Default location */
    obj->duty_raw = 1500;/*Default actual pulse width */
}
