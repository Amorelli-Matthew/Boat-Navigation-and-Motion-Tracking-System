#ifndef WIFI_AP_H
#define WIFI_AP_H


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"

#define EXAMPLE_ESP_WIFI_SSID      "TEST"
#define EXAMPLE_ESP_WIFI_PASS      "TESTDEMO"
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_ENABLED
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN



#define CONNECTED_BIT BIT0
#define FAIL_BIT      BIT1

static EventGroupHandle_t wifi_event_group;


static const char *WIFI_TAG = "wifi softAP";


void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
void wifi_init_softap(void);

#endif
