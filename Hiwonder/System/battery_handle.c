#include "adc.h"
#include "global.h"
#include "global_conf.h"
#include "buzzer.h"
#include "packet_reports.h"
#include "packet.h"

float battery_volt = 0.0f;/*Global variable of battery voltage, unit v */
static uint16_t battery_min_limit = 6300;/*Low voltage alarm value */

static uint16_t adc_value[2];


extern osMessageQueueId_t lvgl_event_queueHandle;

void battery_check_timer_callback(void *argument)
{
    if(adc_value[0] != 0 && adc_value[0] != 4095) {/*The internal reference voltage cannot be 0, otherwise it cannot be calculated */
        //float vdda = 3300.0f *((float)(*((__IO uint16_t*)(0x1FFF7A2A)))) /((float)adc_value[0]);
        //float volt = vdda /4095.0f *((float)adc_value[1]) *11.0f ; /*100k + 10k Resistor voltage divider, the actual voltage is 11 times the measured voltage */
		float volt = 1210.0f / ((float)adc_value[0]) * ((float)adc_value[1]) * 11.0f;
        volt = volt > 20000 ? 0 : volt;/*ADC read value exceeds the maximum allowable power supply voltage, data error */
        battery_volt = battery_volt == 0 ? volt : battery_volt * 0.95f + volt * 0.05f;
    }
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_value, 2);
    static int battery_report_count = 0;
    battery_report_count++;

    if(battery_report_count > (int)(1 * 1000 / BATTERY_TASK_PERIOD)) {/*Send Bluetooth voltage reports regularly */
        battery_report_count = 0;
		PacketReportBatteryVoltageTypeDef report;
		report.sub_cmd = 0x04;
		report.voltage = (int)(battery_volt + 0.5f);
        packet_transmit(&packet_controller, PACKET_FUNC_SYS, &report, sizeof(PacketReportBatteryVoltageTypeDef));

#if ENABLE_OLED
		extern int oled_battery;
		oled_battery = (int)(battery_volt + 0.5f);
#endif
		
#if ENABLE_LVGL
        ObjectTypeDef object;
        object.structure.type_id = OBJECT_TYPE_ID_BATTERY_VOLTAGE;
        *((uint16_t*)object.structure.data) = (int)(battery_volt + 0.5f);
        osMessageQueuePut(lvgl_event_queueHandle, &object, 0, 0);
#endif
    }

#if ENABLE_BATTERY_LOW_ALARM
    static int count = 0;
    if(battery_volt < battery_min_limit && battery_volt > 4900) {
        count++;
    } else {
        count = 0;
    }
    if(count > (int)(10 * 1000 / BATTERY_TASK_PERIOD)) {/*Alarm sound is triggered every 10 seconds */
        buzzer_didi(buzzers[0], 2100, 800, 200, 5);
        count = 0;
    }
#endif
}


void change_battery_limit(uint16_t limit)
{
    battery_min_limit = limit;
}

