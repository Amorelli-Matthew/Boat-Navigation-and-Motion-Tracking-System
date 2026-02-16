#include "EventHandling.h"
#include "esp_adc/adc_oneshot.h"

#include "esp_log.h"

//#include "driver/adc.h"

#define ADC_WIDTH 12
#define ADC_CHANNEL          ADC_CHANNEL_0          

//static adc_oneshot_handle_t adc1_handle;

//adc_oneshot_chan_cfg_t adc_handle;

/*

void Power_Event_Handling(void* pvParameter) {

    // Continuously monitor voltage
    while (1) {
    
        // Delay for 60 seconds before checking again
        vTaskDelay(pdMS_TO_TICKS(60000));  // 60 seconds delay
    }
}


*/


void setup_adc() {

        adc_oneshot_unit_handle_t adc_handle;
    
        adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };


     adc_oneshot_new_unit(&init_config1 ,&adc_handle);

    adc_oneshot_chan_cfg_t cfg = {
        .bitwidth = ADC_WIDTH,    // 12-bit resolution
        .atten = ADC_ATTEN_DB_12       // 0 dB attenuation for 0-3.3V range
    };

    (adc_oneshot_config_channel(adc_handle, ADC_CHANNEL  , &cfg));

     
    int adc_value = 0;     
     float voltage = 0;

    // Continuously monitor voltage
    while (1) {
        // Read ADC value from the internal voltage sensor
    adc_oneshot_read(adc_handle, ADC_CHANNEL , &adc_value);

        // Convert ADC value (12-bit) to voltage, whbere the max ADC value is 4095
        voltage = ( (float) adc_value / 4095.0) * 3.3 *2; // Adjust based on actual reference voltage if needed

        // Log the voltage value
        ESP_LOGI("Power Event", "Current voltage: %fV", voltage);

        // Delay for 60 seconds before checking again
        vTaskDelay(pdMS_TO_TICKS(5000));  // 60 seconds delay
  
  //60000
    }

    

}
