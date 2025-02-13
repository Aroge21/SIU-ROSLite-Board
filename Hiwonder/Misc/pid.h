/**
 *@file pid.h
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief PID-related data structure and method declaration
 *@version 0.1
 *@date 2023-07-12
 *
 *@copyright Copyright (c) 2023
 *
 */

#ifndef _PID_H
#define _PID_H

#include <stdint.h>

/**
  *@brief PID controller structure
  *
  */
typedef struct {
	float set_point;/**< @brief target value */
	float kp;/**< @brief proportional gain */
	float ki;/**< @brief integral gain */
	float kd;/**< @brief Differential Gain */
	
	float previous_0_err;/**< @brief Last error */
	float previous_1_err;/**< @brief Last error */
	
	float output;/**< @brief PID output */
}PID_ControllerTypeDef;



/**
 *@brief PID control update
 *@param self PID controller object pointer
 *@param actual Current actual value
 *@param The interval from last update
 *@retval None.
 */
void pid_controller_update(PID_ControllerTypeDef *self, float actual, float time_delta);



/**
 *@brief Initialize the PID controller
 *@param self PID controller pointer to be initialized
 *@param kp proportional gain
 *@param ki Points Gain
 *@param kd Differential Gain
 *@retval None.
 */
void pid_controller_init(PID_ControllerTypeDef *self, float kp, float ki, float kd);

#endif
