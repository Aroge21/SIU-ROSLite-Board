/**
 * @file app.c
 * @author Lu Yongping (Lucas@hiwonder.com)
 * @brief 主应用逻辑(main appilication logic)
 * @version 0.1
 * @date 2023-05-08
 *
 * @copyright Copyright (c) 2023
 *
 */

#include "cmsis_os2.h"
#include "led.h"
#include "lwmem_porting.h"
#include "global.h"
#include "global.h"
#include "adc.h"
#include "u8g2_porting.h"
#include "packet_reports.h"
#include "packet_handle.h"
#include "serial_servo.h"
#include "rgb_spi.h"
#include "motor_porting.h"
#include "motors_param.h"

void buzzers_init(void);
void buttons_init(void);
void leds_init(void);
void serial_servo_init(void);
void pwm_servos_init(void);
void chassis_init(void);


void button_event_callback(ButtonObjectTypeDef *button,  ButtonEventIDEnum event)
{
    PacketReportKeyEventTypeDef report = {
        .key_id = button->id,
        .event = (uint8_t)(int)event,
    };
    packet_transmit(&packet_controller, PACKET_FUNC_KEY, &report, sizeof(PacketReportKeyEventTypeDef));
	if(event == BUTTON_EVENT_CLICK) {
		buzzer_didi(buzzers[0], 2000, 50, 50, 1);
	}
}

//extern void motor1_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
//extern void motors_init(void);

void app_task_entry(void *argument)
{
	  int i =10;
    extern osTimerId_t led_timerHandle;
    extern osTimerId_t buzzer_timerHandle;
    extern osTimerId_t button_timerHandle;
    extern osTimerId_t battery_check_timerHandle;
    extern osMessageQueueId_t moving_ctrl_queueHandle;

    leds_init();
    motors_init();
    pwm_servos_init();
	  serial_servo_init();
    buzzers_init();
    buttons_init();
	  WS2812b_Configuration();
	  set_chassis_type(CHASSIS_TYPE_TANKBLACK);
    
    button_register_callback(buttons[0], button_event_callback);
    button_register_callback(buttons[1], button_event_callback);
    
    osTimerStart(led_timerHandle, LED_TASK_PERIOD);
    osTimerStart(buzzer_timerHandle, BUZZER_TASK_PERIOD);
    osTimerStart(button_timerHandle, BUTTON_TASK_PERIOD);
    osTimerStart(battery_check_timerHandle, BATTERY_TASK_PERIOD);
    packet_handle_init();
    
//    osDelay(50);
    

    while(1){

		  motor1_set_pulse(motors[1],500);
      HAL_Delay(10000);
		
		  motor1_set_pulse(motors[0],1000);
		  HAL_Delay(10000);
		  motor1_set_pulse(motors[0],500);
		  HAL_Delay(10000);	
		  motor1_set_pulse(motors[0],0);
		  HAL_Delay(10000);			
		  motor1_set_pulse(motors[0],-500);
      HAL_Delay(10000);
		  motor1_set_pulse(motors[0],-1000);
		  HAL_Delay(10000);
		  motor1_set_pulse(motors[0],-500);
		  HAL_Delay(10000);	
    
		  HAL_Delay(10000);
		}
}


