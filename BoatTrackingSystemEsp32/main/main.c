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

        //  ESP_LOGI(TAG, "%s", line);

        // Yield predictably so the watchdog stays happy
        vTaskDelayUntil(&last, pdMS_TO_TICKS(20));
    }

    // Never reached
    // vTaskDelete(NULL);
}

static void gps_task(void *arg)
{

    char gprmcString[128];
    char gpggaString[96];
    char gpvtgString[80];

    TickType_t last = xTaskGetTickCount();

    const TickType_t period = pdMS_TO_TICKS(5000); // 5000 ms

    for (;;)
    {
        generateRandomGPRMC(gprmcString, GPRMCLENGTH); // ensure it writes "\r\n" and '\0'

        //     fix line ending if needed
        size_t n = strnlen(gprmcString, GPRMCLENGTH);

        if (n >= 2 && !(gprmcString[n - 2] == '\r' && gprmcString[n - 1] == '\n'))
        {

            if (n + 2 < sizeof 80)
            {

                if (n >= 2 && !(gprmcString[n - 2] == '\r' && gprmcString[n - 1] == '\n'))
                {

                    if (n + 2 < sizeof gpvtgString)
                    {
                        gprmcString[n++] = '\r';
                        gprmcString[n++] = '\n';
                        gprmcString[n] = '\0';
                    }
                }
            }

            generateRandomGPGGA(gpggaString, GPGGALENGTH);
            generateRandomGPVTG(gpvtgString, GPVTGLENTH);

            //   printf("%s\n%s\n%s\n", gprmcString,gpggaString,gpvtgString);  // print
            //   printf("Size of GpsData: %zu bytes\n", sizeof(GpsData));
            //  printf("%s\n", gprmcString);  // print
            ParseGPRMCMessage(gprmcString, 128);

            vTaskDelayUntil(&last, period);                                // yields reliably; Idle tasks get time, WDT stays happy
            printf("%s\n%s\n%s\n", gprmcString, gpggaString, gpvtgString); // print
                                                                           //  printf("%s\n", gpggaString);  // print

            // vTaskDelayUntil(&last, period);  // yields reliably; Idle tasks get time, WDT stays happy
            vTaskDelete(NULL);
        }
    }
}
void app_main(void)
{

    // BluetoothInit();
    ESP_LOGI(TAG, "GPS SIZE: %u", getSizeOfGpsVar());
    ESP_LOGI(TAG, "Alignment: %lu\n", __alignof__(GpsData));

    // //bluetooth is not a freertos task!
    Bluetooth_task();

    // xTaskCreatePinnedToCore(virtual_gps_task, "GPS_sensor", 4096, NULL, 1, NULL, 1);

    //  xTaskCreatePinnedToCore(GPS_sensor, "gps_task", 4096, NULL, 1, NULL, 1);

    xTaskCreatePinnedToCore(GPS_sensor, "GPS_sensor", 4096, NULL, 1, NULL, 1);
}
