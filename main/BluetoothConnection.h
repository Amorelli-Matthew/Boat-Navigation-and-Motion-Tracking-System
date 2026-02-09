#ifndef BLUETOOTHCONNECTION_H
#define BLUETOOTHCONNECTION_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "nvs_flash.h"
#include <string.h>

//bluetooth
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "esp_random.h"

#define PROFILE_NUM 3
#define PROFILE_GPSLocation_APP_ID 0
#define PROFILE_CompassMotionSensor_APP_ID 1
#define PROFILE_Controller_APP_ID 2

//change??
#define HEART_RATE_SVC_UUID 0x180D
#define HEART_RATE_CHAR_UUID 0x2A37
#define HEART_NUM_HANDLE 4
#define AUTO_IO_SVC_UUID 0x1815
#define AUTO_IO_NUM_HANDLE 3

#define ADV_CONFIG_FLAG      (1 << 0)
#define SCAN_RSP_CONFIG_FLAG (1 << 1)
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    uint16_t service_handle;
    esp_gatt_srvc_id_t service_id;
    uint16_t char_handle;
    esp_bt_uuid_t char_uuid;
    esp_gatt_perm_t perm;
    esp_gatt_char_prop_t property;
    uint16_t descr_handle;
    esp_bt_uuid_t descr_uuid;
};




void init_bluetooth_attributes(void);
 void Bluetooth_task();

 void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

 void GPSLocation_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
 void auto_io_gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
 void example_write_event_env(esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
 void heart_rate_task(void* param);
#endif