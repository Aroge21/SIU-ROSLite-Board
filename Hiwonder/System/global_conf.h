#ifndef __GLOBAL_CONF_H
#define __GLOBAL_CONF_H

#define ENABLE_IMU  1                       /*Whether the IMU task is started */
#define ENABLE_LVGL 0                       /*Whether the LVGL task is started */
#define ENABLE_OLED 1                       /*Whether to enable OLED display */
#define ENABLE_BATTERY_LOW_ALARM 1          /*Is the low voltage alarm turned on */

#define KEY1_PUSHED_LEVEL 0
#define KEY2_PUSHED_LEVEL 0
#define LED_SYS_LEVEL_ON  0

#define LED_TASK_PERIOD     30u             /*LED status refresh interval */
#define BUZZER_TASK_PERIOD  30u             /*Buzzer status refresh interval */
#define BUTTON_TASK_PERIOD  30u             /*Onboard key scanning interval */
#define BATTERY_TASK_PERIOD 50u             /*Battery capacity detection interval */

#endif

