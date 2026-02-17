#ifndef EVENTHANDLING_H
#define EVENTHANDLING_H

#include <stdio.h>
#include "esp_system.h"        
#include "esp_log.h"           

#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"     
#include "esp_adc/adc_oneshot.h"

#define INTERNAL_VOLTAGE_SENSOR ADC1_CHANNEL_0  // Use ADC1_CHANNEL_0 (GPIO36) to measure internal voltage
#define V_REF 1100  // Internal reference voltage (in mV)

//esp_adc_oneshot_handle_t adc1_handle; // Corrected handle type

void Power_Event_Handling(void* pvParameter);
void setup_adc();

#endif
