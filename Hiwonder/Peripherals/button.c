/**
 *@file button.c
 *@author Lu Yongping (Lucas@hiwonder.com)
 *@brief implements hardware-independent key detection
 *@version 0.1
 *@date 2023-05-08
 *
 *@copyright Copyright (c) 2023
 *
 */
#include "button.h"


void button_task_handler(ButtonObjectTypeDef *self, uint32_t period)
{
    self->ticks_count += period;

    uint32_t pin = self->read_pin(self);
    if(pin != self->last_pin_raw)  {/*The two consecutive IO states are different before and after, and the button state is considered to be unstable. Save the new IO state and return */
        self->last_pin_raw = pin;
        return;
    }
	
/*The button state has not changed, that is, the state machine state will not transfer, and it will be returned directly */
    if(self->last_pin_filtered == self->last_pin_raw && self->stage != BUTTON_STAGE_PRESS && self->stage != BUTTON_STAGE_LONGPRESS) { 
        return;
    }

    self->last_pin_filtered = self->last_pin_raw;/*Save new button status */
    switch(self->stage) {
        case BUTTON_STAGE_NORMAL: {
            if(self->last_pin_filtered) {
                self->event_callback(self, BUTTON_EVENT_PRESSED);/*Trigger key press event */
                if(self->ticks_count < self->combin_th && self->combin_counter > 0) {/*Combo will only work if the combo count is not zero */
                    self->combin_counter += 1;
                    if(self->combin_counter == 2) {/*Double-click the callback */
                        self->event_callback(self, BUTTON_EVENT_DOUBLE_CLICK);
                    }
                    if(self->combin_counter == 3) {/*Three-combo callback */
                        self->event_callback(self, BUTTON_EVENT_TRIPLE_CLICK);
                    }
                }
                self->ticks_count = 0;
                self->stage = BUTTON_STAGE_PRESS;
            } else {
                if(self->ticks_count > self->combin_th && self->combin_counter != 0) {
                    self->combin_counter = 0;
                    self->ticks_count = 0;
                }
            }
            break;
		}
        case BUTTON_STAGE_PRESS: {
            if(self->last_pin_filtered) {
                if(self->ticks_count > self->lp_th) {/*Long press trigger time exceeds */
                    self->event_callback(self, BUTTON_EVENT_LONGPRESS);/*Trigger long press event */
                    self->ticks_count = 0;
                    self->stage = BUTTON_STAGE_LONGPRESS;/*status changes to long press */
                }
            } else {/*button release */
                self->event_callback(self, BUTTON_EVENT_RELEASE_FROM_SP);/*Trigger the short press release event */
                self->event_callback(self, BUTTON_EVENT_CLICK);/*Trigger click release event */
                self->combin_counter = self->combin_counter == 0 ? 1 : self->combin_counter;/*Combo will only work if the combo count is not zero */
                self->stage = BUTTON_STAGE_NORMAL;
            }
            break;
		}
        case BUTTON_STAGE_LONGPRESS: {
            if(self->last_pin_filtered) {
                if(self->ticks_count > self->repeat_th)  {
                    self->event_callback(self, BUTTON_EVENT_LONGPRESS_REPEAT);/*Trigger the long press and repeat the event */
                    self->ticks_count = 0;/*Re-timed the next repeated trigger */
                }
            } else {/*button release */
                self->event_callback(self, BUTTON_EVENT_RELEASE_FROM_LP);/*Trigger the long press release event */
                self->combin_counter = 0;/*Press and hold it long to not combo, the combo count will not take effect when the combo count is 0 */
                self->ticks_count = self->combin_th + 1;/*Press and hold it long to not combo, so that the combo timeout will be directly timed out */
                self->stage = BUTTON_STAGE_NORMAL;
            }
            break;
		}
    }
}


void button_register_callback(ButtonObjectTypeDef *self, ButtonEventCallbackFuncTypeDef callback)
{
    if(NULL == callback) {
        return;
    }
    self->event_callback = callback;
}

void button_defalut_event_callback(ButtonObjectTypeDef *self,  ButtonEventIDEnum event)
{
}

void button_object_init(ButtonObjectTypeDef *self)
{
    self->stage = BUTTON_STAGE_NORMAL;
    self->last_pin_raw = 0;
	self->last_pin_filtered = 0;
    self->combin_counter = 0;
    self->ticks_count = 0;

/*config */
    self->combin_th = 400; 
    self->lp_th = 2000;  
    self->repeat_th = 500;
    self->event_callback = button_defalut_event_callback;
}

