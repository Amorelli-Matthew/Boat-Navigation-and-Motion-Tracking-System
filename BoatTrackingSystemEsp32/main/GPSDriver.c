#include "GPSDriver.h"
#include <string.h>
#include "esp_log.h"

static const char *TAG = "UBX_PARSER";

static ubx_state_t state = STATE_IDLE;
static uint8_t msg_class, msg_id, ck_a, ck_b;
static uint16_t msg_len, payload_idx;
static uint8_t payload_buffer[UBX_MAX_PAYLOAD];

static GpsData CurrentGpsData;

static int gpsInterval = 2 * 1000 ; //ms


//creates shared instance
SemaphoreHandle_t gps_mutex;

// UBX-CFG-VALSET to enable NAV-PVT on UART1
//will be used on real hardware but for now commented
// static  uint8_t enable_pvt[] = {
//      0xB5, 0x62, 0x06, 0x8A, 0x09, 0x00, 0x00, 0x01, 0x00, 0x00, 
//      0x07, 0x00, 0x91, 0x20, 0x01, 0x4E, 0x7E
//  };

 double lat, lon;
 float speed_kmh;


 void InitMux()
{
    gps_mutex = xSemaphoreCreateMutex();
}

uint8_t* get_gps_data_ptr(void) {
    return (uint8_t*)&CurrentGpsData;
}

GpsData getData()
{
    return CurrentGpsData;
}

int getSizeOfGpsVar()
{
    return (int)sizeof(CurrentGpsData);
}
void setGpsInterval(int new_interval)
{
    if (new_interval < 100) new_interval = 100; // Minimum 100ms safety
    if (new_interval > 60000) new_interval = 60000; // Maximum 1 minute safety
    gpsInterval = new_interval;
}

int getGpsInterval(void)
{
    return gpsInterval;
}

// Checksum calculation: Fletcher-8
// Calculates over Class, ID, Length, and Payload
void calculate_mock_checksum(uint8_t *data, size_t len, uint8_t *ck_a, uint8_t *ck_b) {
    uint8_t a = 0, b = 0;
    for (size_t i = 2; i < len - 2; i++) { 
        a += data[i];
        b += a;
    }
    *ck_a = a; 
    *ck_b = b;
}



 //method that parses a nav pvt message
void parse_UBX_NAV_PVT_message(uint8_t m_class, uint8_t m_id, uint8_t *payload, uint16_t len)
 {
    //method that checks to see if the  message is a nav pvt id
    if (m_class == UBX_NAV_CLASS && m_id == UBX_NAV_PVT_ID) 
    {
        //check if the length is equal to 92 bytes
        if (len == UBX_NAV_PVT_MAXSIZE) {
            
            //copy the payload into a pice of ram
            memcpy(&CurrentGpsData, payload, sizeof(GpsData));

            // Check bit 0 of flags for a valid GNSS Fix
            if (CurrentGpsData.flags & 0x01) {
                
                lat = CurrentGpsData.lat / 10000000.0;
                lon = CurrentGpsData.lon / 10000000.0;
                speed_kmh = (CurrentGpsData.gSpeed / 1000.0f) * 3.6f;

                ESP_LOGI(TAG, "FIX OK | Lat: %.7f, Lon: %.7f | Sats: %d | Spd: %.1f km/h", 
                         lat, lon, CurrentGpsData.numSV, speed_kmh);
            } else {
                ESP_LOGW(TAG, "No Fix (Sats: %d, Type: %d)", CurrentGpsData.numSV, CurrentGpsData.fixType);
            }
        }
    }
    
    if (m_class == 0x05)
    {
        if (m_id == 0x01) ESP_LOGI("UBX", "Command ACK-nowledged");

        else if (m_id == 0x00) ESP_LOGE("UBX", "Command NACK-ed (Rejected)");
    }
    
}


void parse_ubx_byte(uint8_t byte) {
    switch (state) {
        case STATE_IDLE:
            if (byte == UBX_SYNC1) state = STATE_SYNC1;
            break;

        case STATE_SYNC1:
            if (byte == UBX_SYNC2) {
                state = STATE_SYNC2;
                ck_a = 0; ck_b = 0;
            } else {
                state = STATE_IDLE;
            }
            break;

        case STATE_SYNC2:
            msg_class = byte;
            ck_a += byte; ck_b += ck_a;
            state = STATE_CLASS;
            break;

        case STATE_CLASS:
            msg_id = byte;
            ck_a += byte; ck_b += ck_a;
            state = STATE_ID;
            break;

        case STATE_ID:
            msg_len = byte; // LSB
            ck_a += byte; ck_b += ck_a;
            state = STATE_LEN_L; 
            break;

        case STATE_LEN_L:
            msg_len |= (byte << 8); // MSB
            ck_a += byte; ck_b += ck_a;
            payload_idx = 0;
            
            if (msg_len > UBX_MAX_PAYLOAD) {
                state = STATE_IDLE;
            } else if (msg_len == 0) {
                state = STATE_CHKA;
            } else {
                 //Ready to parse
                state = STATE_PAYLOAD;
            }
            break;

        case STATE_PAYLOAD:
            payload_buffer[payload_idx++] = byte;
            ck_a += byte; ck_b += ck_a;
            if (payload_idx >= msg_len) state = STATE_CHKA;
            break;

        case STATE_CHKA:
            if (byte == ck_a) state = STATE_CHKB;
            else {
                ESP_LOGE(TAG, "Checksum A error (Exp: 0x%02X, Got: 0x%02X)", ck_a, byte);
                state = STATE_IDLE;
            }
            break;

        case STATE_CHKB:
            if (byte == ck_b) {
                parse_UBX_NAV_PVT_message(msg_class, msg_id, payload_buffer, msg_len);
            } else {
                ESP_LOGE(TAG, "Checksum B error (Exp: 0x%02X, Got: 0x%02X)", ck_b, byte);
            }
            state = STATE_IDLE;
            break;

        default:
            state = STATE_IDLE;
            break;
    }
}