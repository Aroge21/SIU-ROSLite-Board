/**
 *@file led.c
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief implements hardware-independent LED light flash control
 *@version 0.1
 *@date 2023-05-13
 *
 *@copyright Copyright (c) 2023
 *
 */

#include "led.h"

void led_task_handler(LEDObjectTypeDef *self, uint32_t period)
{
/*Try to get new control data from the queue, if it is successfully fetched, reset the state machine to restart a control loop */
    if(self->get_ctrl_block(self, &self->ctrl_structure) == 0) {
        self->stage = LED_STAGE_START_NEW_CYCLE;
    }
/*State machine processing */
    switch(self->stage) {
        case LED_STAGE_START_NEW_CYCLE: {
            if(self->ctrl_structure.ticks_on > 0) {
                self->set_pin(self, 1);
                if(self->ctrl_structure.ticks_off > 0) {/*If the expiration time is not 0, it will flash otherwise it will be long */
                    self->ticks_count = 0;
                    self->stage = LED_STAGE_WATTING_OFF;/*Wait for the LED light to turn on time end */
                }else{
					self->stage = LED_STAGE_IDLE;/*Long bright, turn to idle */
				}
            } else {/*As long as the light-up time is 0, it will be long-destructed */
                self->set_pin(self, 0);
				self->stage = LED_STAGE_IDLE;/*Long-term destruction, turn to idle */
            }
            break;
        }
        case LED_STAGE_WATTING_OFF: {
            self->ticks_count += period;
            if(self->ticks_count >= self->ctrl_structure.ticks_on) {/*LED light up time ends */
                self->set_pin(self, 0);
                self->stage = LED_STAGE_WATTING_PERIOD_END;
            }
            break;
        }
        case LED_STAGE_WATTING_PERIOD_END: {/*Waiting for the end of the cycle */
            self->ticks_count += period;
            if(self->ticks_count >= (self->ctrl_structure.ticks_off + self->ctrl_structure.ticks_on)) {
				 self->ticks_count -= (self->ctrl_structure.ticks_off + self->ctrl_structure.ticks_on);
                if(self->ctrl_structure.repeat == 1) {/*This control task can be ended when the remaining number of repetitions is 1 */
                    self->set_pin(self, 0);
                    self->stage = LED_STAGE_IDLE;/*Use up the number of repetitions, turn to idle */
                } else {
                    self->set_pin(self, 1);
                    self->ctrl_structure.repeat = self->ctrl_structure.repeat == 0 ? 0 : self->ctrl_structure.repeat - 1;
                    self->stage = LED_STAGE_WATTING_OFF;
                }
            }
            break;
        }
        case LED_STAGE_IDLE: {
            break;
        }
        default:
            break;
    }
}

int led_on(LEDObjectTypeDef *self)
{
    LEDCtrlTypeDef ctrl_structure = {
        .ticks_on = 1,
        .ticks_off = 0,
        .repeat = 0,
    };
    return self->put_ctrl_block(self, &ctrl_structure);
}

int led_off(LEDObjectTypeDef *self)
{
    LEDCtrlTypeDef ctrl_structure = {
        .ticks_on = 0,
        .ticks_off = 0,
        .repeat = 0,
    };
    return self->put_ctrl_block(self, &ctrl_structure);
}

int led_flash(LEDObjectTypeDef *self, uint32_t ticks_on, uint32_t ticks_off, uint32_t repeat)
{
    LEDCtrlTypeDef ctrl_structure = {
        .ticks_on = ticks_on,
        .ticks_off = ticks_off,
        .repeat = repeat,
    };
    return self->put_ctrl_block(self, &ctrl_structure);
}


void led_object_init(LEDObjectTypeDef *self)
{
    self->stage = LED_STAGE_IDLE;
}


