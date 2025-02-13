/**
 * @file buzzer.c
 * @author 
 * @brief Implementation of hardware & system-independent buzzer control
 * @version 0.1
 * @date 2023-05-18
 *
 * @copyright Copyright (c) 2023
 *
 */
#include "buzzer.h"

void buzzer_task_handler(BuzzerObjectTypeDef *self, uint32_t period)
{
    /* Try to get new control data from the queue, if successful, reset the state machine to start a new control cycle */
    if(self->get_ctrl_block(self, &self->ctrl_structure) == 0) {
        self->stage = BUZZER_STAGE_START_NEW_CYCLE;
    }
    /* State machine handling */
    switch(self->stage) {
        case BUZZER_STAGE_START_NEW_CYCLE: {
            if(self->ctrl_structure.ticks_on > 0 && self->ctrl_structure.freq > 0) {
                self->set_pwm(self, self->ctrl_structure.freq); /* Sound the buzzer */
                if(self->ctrl_structure.ticks_off > 0) { /* If the silent time is not 0, it will beep, otherwise it will be a continuous sound */
                    self->ticks_count = 0;
                    self->stage = BUZZER_STAGE_WAITING_OFF; /* Wait for the sound time to end */
                } else {
                    self->stage = BUZZER_STAGE_IDLE; /* Continuous sound, switch to idle */
                }
            } else { /* As long as the sound time is 0, it will be silent */
                self->set_pwm(self, 0);
                self->stage = BUZZER_STAGE_IDLE;  /* Long silence, switch to idle */
            }
            break;
        }
        case BUZZER_STAGE_WAITING_OFF: {
            self->ticks_count += period;
            if(self->ticks_count >= self->ctrl_structure.ticks_on) { /* Sound time ends */
                self->set_pwm(self, 0);
                self->stage = BUZZER_STAGE_WAITING_PERIOD_END;
            }
            break;
        }
        case BUZZER_STAGE_WAITING_PERIOD_END: { /* Wait for the period to end */
            self->ticks_count += period;
            if(self->ticks_count >= (self->ctrl_structure.ticks_off + self->ctrl_structure.ticks_on)) {
                self->ticks_count -= (self->ctrl_structure.ticks_off + self->ctrl_structure.ticks_on);
                if(self->ctrl_structure.repeat == 1) { /* When the remaining repeat count is 1, the control task can be ended */
                    self->set_pwm(self, 0);
                    self->stage = BUZZER_STAGE_IDLE;
                } else {
                    self->set_pwm(self, self->ctrl_structure.freq);
                    self->ctrl_structure.repeat = self->ctrl_structure.repeat == 0 ? 0 : self->ctrl_structure.repeat - 1;
                    self->stage = BUZZER_STAGE_WAITING_OFF;
                }
            }
            break;
        }
        case BUZZER_STAGE_IDLE: {
            break;
        }
        default:
            break;
    }
}


int buzzer_on(BuzzerObjectTypeDef *self, uint32_t freq)
{
    BuzzerCtrlTypeDef ctrl_structure = {
        .freq = freq,
        .ticks_on = 1,  
        .ticks_off = 0, /* Silent time is 0, continuous sound */
        .repeat = 0,
    };
    return self->put_ctrl_block(self, &ctrl_structure);
}

int buzzer_off(BuzzerObjectTypeDef *self)
{
    BuzzerCtrlTypeDef ctrl_structure = {
        .freq = 0,
        .ticks_on = 0, /* Sound time is 0, silent */
        .ticks_off = 0,
        .repeat = 0,
    };
    return self->put_ctrl_block(self, &ctrl_structure);
}

int buzzer_didi(BuzzerObjectTypeDef *self, uint32_t freq, uint32_t ticks_on, uint32_t ticks_off, uint32_t repeat)
{
    BuzzerCtrlTypeDef ctrl_structure = {
        .freq = freq,
        .ticks_on = ticks_on,
        .ticks_off = ticks_off,
        .repeat = repeat,
    };
    return self->put_ctrl_block(self, &ctrl_structure);
}


void buzzer_object_init(BuzzerObjectTypeDef *self)
{
    self->stage = BUZZER_STAGE_IDLE;
}
