#ifndef BLUETOOTHCONNECTION_H
#define BLUETOOTHCONNECTION_H
#include "GPSDriver.h"

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
#include "nimble/ble.h"
#include "modlog/modlog.h"
#include "esp_peripheral.h"

#include <assert.h>
#include "host/ble_hs.h"
#include "host/ble_uuid.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "services/ans/ble_svc_ans.h"

#include "esp_random.h"

#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/util/util.h"
#include "console/console.h"

#define GATT_SVR_SVC_ALERT_UUID               0x1811
#define GATT_SVR_CHR_SUP_NEW_ALERT_CAT_UUID   0x2A47
#define GATT_SVR_CHR_NEW_ALERT                0x2A46
#define GATT_SVR_CHR_SUP_UNR_ALERT_CAT_UUID   0x2A48
#define GATT_SVR_CHR_UNR_ALERT_STAT_UUID      0x2A45
#define GATT_SVR_CHR_ALERT_NOT_CTRL_PT        0x2A44

#define MAX_NOTIFY 5

struct ble_hs_cfg;
struct ble_gatt_register_ctxt;

void BluetoothInit();
 void Bluetooth_task();

void gatt_svr_register_cb(struct ble_gatt_register_ctxt *ctxt, void *arg);
int gatt_svr_init(void);


#endif