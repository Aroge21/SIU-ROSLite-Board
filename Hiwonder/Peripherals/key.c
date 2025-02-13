///Implement hardware-independent key detection
#include "global.h"
#include "key.h"

static void default_callback(KeyObjectTypeDef *key, KeyEventEnum event)
{
//   const char events[8][64] = {
//       "KEY_EVENT_PRESSED",
//       "KEY_EVENT_LONGPRESS",
//       "KEY_EVENT_LONGPRESS_REPEAT",
//       "KEY_EVENT_RELEASE_FROM_LP",
//       "KEY_EVENT_RELEASE_FROM_SP",
//       "KEY_EVENT_CLICK",
//       "KEY_EVENT_DOUBLE_CLICK",
//       "KEY_EVENT_TRIPLE_CLICK",
//   };
//   printf("event: %s\r\n", (char*)&events[event]);
}

void key_refresh(KeyObjectTypeDef *self)
{
    uint8_t level = self->read_pin();
    uint32_t current_tick = self->get_ticks();
    if(level == self->last_level) {//The original pin states are consistently believed to be stable and non-error triggered by the key state.
        switch(self->state) {
        case KEY_STATE_NORMAL:
            if(level == self->level_press) {
/*Trigger key press event */
                if(NULL != self->event_callback) {
                    self->event_callback(self, KEY_EVENT_PRESSED);
                }
                if((current_tick - self->stamp) < self->combin_th) {
                    self->combin_counter += 1;
                    if(NULL != self->event_callback) {
                        if(self->combin_counter == 1) {
                            self->event_callback(self, KEY_EVENT_DOUBLE_CLICK);
                        }
                        if(self->combin_counter == 2) {
                            self->event_callback(self, KEY_EVENT_TRIPLE_CLICK);
                        }
                    }
                }
                self->stamp = current_tick;/*Note the time of this press */
                self->state = KEY_STATE_PRESS;
            } else {
                if((current_tick - self->stamp) > self->combin_th && self->combin_counter != 0) {
                    self->combin_counter = 0;
                }
            }
            break;
        case KEY_STATE_PRESS:
            if(level != self->level_press) {//If the button is not pressed, it is released
                /*Trigger the short press release event */
                if(NULL != self->event_callback) {
                    self->event_callback(self, KEY_EVENT_RELEASE_FROM_SP);
                    self->event_callback(self, KEY_EVENT_CLICK);
                }
                self->state = KEY_STATE_NORMAL;
            } else {
/*If the long press trigger time exceeds the long press, the long press event will be triggered, and the status will change to long press*/
                if((current_tick - self->stamp) > self->lp_th) {
                    self->event_callback(self, KEY_EVENT_LONGPRESS);
                    self->state = KEY_STATE_LONGPRESS;
                    self->combin_counter = 0;
                    self->stamp = current_tick;/*Note the first long press trigger time */
                }
            }
            break;
        case KEY_STATE_LONGPRESS:
            if(level != self->level_press) {//If the button is not pressed, it is released
                /*Trigger the long press release event */
                if(NULL != self->event_callback) {
                    self->event_callback(self, KEY_EVENT_RELEASE_FROM_LP);
                    self->stamp = 0;
                }
                self->state = KEY_STATE_NORMAL;
            } else {
                if((current_tick - self->stamp) > self->repeat_th)  {
                    self->event_callback(self, KEY_EVENT_LONGPRESS_REPEAT);
                    self->stamp = current_tick;/*Note the retrigger time this time */
                }
            }
            break;
        default:
            while(1) {
                printf("KEY REFRESH ERROR!!!\r\n");
            }
        }
    }
    self->last_level = level;
}


void key_obj_init(KeyObjectTypeDef *self)
{
    self->state = KEY_STATE_NORMAL;
    self->last_level = !self->level_press;
    self->refresh = key_refresh;
    self->event_callback = default_callback;
}

