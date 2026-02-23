#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "EventHandling.h"
#include "GPSDriver.h"
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"

// my custom librarys
#include "BluetoothConnection.h"

#define GPRMCLENGTH 128
#define GPGGALENGTH 96
#define GPVTGLENTH 80

#define UART_NUM 0
#define NO_TX_PIN 16

typedef struct
{
    const uart_port_t uart_num;
    uart_config_t uart_config;

    int rx_pin; // = 16;
    // no tx pin as the gps never recives any data so save memeory!

    int length;
    const int uart_buffer_size;
} GPS_Parms;

static const char *TAG = "GPS";

static void GPS_sensor(void *pv)
{
    setvbuf(stdout, NULL, _IONBF, 0);

    GPS_Parms GPSParms = {
        .uart_num = UART_NUM_1,
        .uart_config = {
            .baud_rate = 9600,
            .data_bits = UART_DATA_8_BITS,
            .parity = UART_PARITY_DISABLE,
            .stop_bits = UART_STOP_BITS_1,
            .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
            .source_clk = UART_SCLK_DEFAULT,
        },
        //.tx_pin = 17,
        .rx_pin = 16,
        .length = 0,
        .uart_buffer_size = 2048};

    // Install and configure UART
    //
    ESP_ERROR_CHECK(uart_driver_install(
        GPSParms.uart_num,
        GPSParms.uart_buffer_size,
        0, 0, NULL, 0));

    ESP_ERROR_CHECK(uart_param_config(GPSParms.uart_num, &GPSParms.uart_config));
    ESP_ERROR_CHECK(uart_set_pin(
        GPSParms.uart_num,
        NO_TX_PIN,
        GPSParms.rx_pin,
        UART_PIN_NO_CHANGE,
        UART_PIN_NO_CHANGE));

    // Flush any partial bytes to start on a clean boundary
    uart_flush_input(GPSParms.uart_num);

    // NMEA line assembly (no checksum)
    char line[256];
    size_t line_len = 0;
    bool in_sentence = false;

    // buffer for when you actually read in data being sent in to esp32
    uint8_t rxbuf[128];
    TickType_t last = xTaskGetTickCount();

    for (;;)
    {
        // Read in chunks (timeout ~50 ms)
        int n = uart_read_bytes(GPSParms.uart_num, rxbuf, sizeof(rxbuf), pdMS_TO_TICKS(50));

        if (n > 0)
        {
            for (int i = 0; i < n; i++)
            {
                char c = (char)rxbuf[i];

                if (!in_sentence)
                {
                    if (c == '$')
                    {
                        in_sentence = true;
                        line_len = 0;
                        line[line_len++] = c; // start with '$'
                    }
                    continue;
                }
                if (c == '\r')
                {
                    // ignore CR; wait for LF to terminate
                    continue;
                }
                else if (c == '\n')
                {
                    // End of sentence
                    if (line_len > 0 && line_len < sizeof(line))
                    {
                        line[line_len] = '\0';

                        // Print the full NMEA sentence
                        printf("%s\n", line);
                    }
                    in_sentence = false;
                    line_len = 0;
                }
                else
                {
                    // Append, guarding against overflow
                    if (line_len < sizeof(line) - 1)
                    {
                        line[line_len++] = c;
                    }
                    else
                    {
                        // Overflow -> reset and wait for next '$'
                        in_sentence = false;
                        line_len = 0;
                    }
                }
            }
        }

        // Yield predictably so the watchdog stays happy
        vTaskDelayUntil(&last, pdMS_TO_TICKS(20));
    }

}

void EmulateGPStask(void *pv) {
    // Seed for true randomness
    srand(time(NULL));

    GpsData fake_pvt = {0};
    fake_pvt.lat = 407127760;   
    fake_pvt.lon = -740059740;  
    fake_pvt.gSpeed = 5500; 

    while (1) {
        //Position & Speed Drift
        fake_pvt.lat += (rand() % 41) - 20; 
        fake_pvt.lon += (rand() % 41) - 20;
        
        int32_t speed_drift = (rand() % 1001) - 500;
        if (fake_pvt.gSpeed + speed_drift > 0) fake_pvt.gSpeed += speed_drift;

        //Signal Quality Simulation
        // 10% chance to lose fix (simulating a tunnel or bridge)
        bool has_fix = (rand() % 100) > 10; 
        
        if (has_fix) {
            fake_pvt.fixType = 3;      // 3D Fix
            fake_pvt.flags = 0x01;     // gnssFixOK = 1
            fake_pvt.numSV = 8 + (rand() % 11);
        } else {
            fake_pvt.fixType = 0;      // No Fix
            fake_pvt.flags = 0x00;     // gnssFixOK = 0
            fake_pvt.numSV = rand() % 4; // 0-3 satellites (not enough for 3D fix)
        }

        fake_pvt.iTOW += 1000;

        //Packet Assembly
        const size_t payload_len = sizeof(fake_pvt);
        uint8_t packet[6 + payload_len + 2]; 
        
        packet[0] = UBX_SYNC1; 
        packet[1] = UBX_SYNC2;
        packet[2] = UBX_NAV_CLASS; 
        packet[3] = UBX_NAV_PVT_ID;
        packet[4] = (payload_len & 0xFF); 
        packet[5] = (payload_len >> 8) & 0xFF;
        
        memcpy(&packet[6], &fake_pvt, payload_len);

        uint8_t ca, cb;
        calculate_mock_checksum(packet, sizeof(packet), &ca, &cb);
        packet[sizeof(packet) - 2] = ca; 
        packet[sizeof(packet) - 1] = cb;

        //grab the shared object
       //if (xSemaphoreTake(gps_mutex, pdMS_TO_TICKS(10)) == pdTRUE) {
        // Feed to parser
        for (int i = 0; i < sizeof(packet); i++) {
            parse_ubx_byte(packet[i]); 
        }

        //        xSemaphoreGive(gps_mutex);

        //}

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void app_main(void)
{
    //InitMux();
    
    // //bluetooth is not a freertos task!
   // Bluetooth_task();

    xTaskCreate(EmulateGPStask, "MockGPS", 4096, NULL, 5, NULL);

}


