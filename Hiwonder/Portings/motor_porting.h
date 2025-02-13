#ifndef __MOTOR_PORTING_H_
#define __MOTOR_PORTING_H_

#include "encoder_motor.h"
#include "motors_param.h"

void set_motor_param(EncoderMotorObjectTypeDef *motor, int32_t tpc, float rps_limit, float kp, float ki, float kd);
void set_motor_type(EncoderMotorObjectTypeDef *motor, MotorTypeEnum type);
void motors_init(void);
void motor1_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
void motor2_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
void motor3_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
void motor4_set_pulse(EncoderMotorObjectTypeDef *self, int speed);


#endif
