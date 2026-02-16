    #include "wifi_ap.h"

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    switch (event_id) {
        case WIFI_EVENT_AP_STADISCONNECTED: {
            // Cast event data to the appropriate type for disconnection event
            wifi_event_sta_disconnected_t* disconnection_event = (wifi_event_sta_disconnected_t*)event_data;

            // Create an array to store the MAC address
            uint8_t mac[6];

            // Get the MAC address of the station that disconnected
            esp_wifi_get_mac(WIFI_IF_STA, mac);

            // Log the MAC address of the device that disconnected
            ESP_LOGI("Wi-Fi", "Station disconnected from AP. MAC: " MACSTR, MAC2STR(mac));
            
            switch (disconnection_event->reason)
            {
                // Environmental issues (e.g., signal loss, weather, interference)
                case WIFI_REASON_BEACON_TIMEOUT:
                case WIFI_REASON_NO_AP_FOUND:
                case WIFI_REASON_AUTH_FAIL:
                case WIFI_REASON_HANDSHAKE_TIMEOUT:
                                    ESP_LOGW(WIFI_TAG, "Emergency: Disconnection reason related to environmental issues or signal instability.");
            
                                    //save the gps data to sdcard
            
                                    break;
            
            default:
              break;
            };
            
            break;
        }
        default:
            break;
    }
}

            // if (event_id == WIFI_EVENT_AP_STACONNECTED) {
            //     wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
            //     ESP_LOGI(WIFI_TAG, "station "MACSTR" join, AID=%d",
            //              MAC2STR(event->mac), event->aid);
            // } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
            //     wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
            //     ESP_LOGI(WIFI_TAG, "station "MACSTR" leave, AID=%d, reason=%d",
            //              MAC2STR(event->mac), event->aid, event->reason);
            //     }
    

    void wifi_init_softap(void)
    {

        ESP_ERROR_CHECK(esp_netif_init());
        ESP_ERROR_CHECK(esp_event_loop_create_default());
        esp_netif_create_default_wifi_ap();
    
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    
        ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                            ESP_EVENT_ANY_ID,
                                                            &wifi_event_handler,
                                                            NULL,
                                                            NULL));
    
                        wifi_config_t wifi_config = {
                          .ap = {
                         .ssid = EXAMPLE_ESP_WIFI_SSID,
                                .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
                          .channel = EXAMPLE_ESP_WIFI_CHANNEL,
                            .password = EXAMPLE_ESP_WIFI_PASS,
                           .max_connection = EXAMPLE_MAX_STA_CONN,
                             .authmode = WIFI_AUTH_WPA3_PSK,  // Explicit authentication mode
                             .pmf_cfg = {
                              .required = true,           // Protected Management Frames
                             .capable = true,
                                  },
                                },
                                   };


        //set the mode, config and start the wifi driver
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());
    
        ESP_LOGI(WIFI_TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
   
    //event handlers for wifi             
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_event_handler, NULL));
    }