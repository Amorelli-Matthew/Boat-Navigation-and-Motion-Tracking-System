set(srcs "main.c" "GPSDriver.c"
        )
         
         idf_component_register(
            SRCS ${srcs}
            INCLUDE_DIRS "."
            REQUIRES 
                esp_timer
                driver
                nvs_flash
                esp_event
                lwip
                freertos
                esp_adc
                esp_driver_uart
<<<<<<< HEAD:BoatTrackingSystemEsp32/main/CMakeLists.txt
                nimble_peripheral_utils
                )
=======
                
                
        )
>>>>>>> parent of 520adba (started bluetooth implimentation and parsing gps sentences):main/CMakeLists.txt
