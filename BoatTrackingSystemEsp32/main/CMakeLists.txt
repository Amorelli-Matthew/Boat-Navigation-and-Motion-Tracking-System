set(srcs "main.c" "GPSDriver.c" "BluetoothConnection.c" 
        )
         
         idf_component_register(
            SRCS ${srcs}
            INCLUDE_DIRS "."
            REQUIRES 
                bt
                esp_timer
                driver
                nvs_flash
                esp_event
                lwip
                freertos
                esp_adc
                esp_driver_uart
                nimble_peripheral_utils
                )
        
