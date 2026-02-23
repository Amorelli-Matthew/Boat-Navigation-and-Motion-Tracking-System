#ifndef GPSDRIVER_H
#define GPSDRIVER_H

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


// UBX Frame Constants
#define UBX_SYNC1       0xB5
#define UBX_SYNC2       0x62
#define UBX_NAV_CLASS   0x01
#define UBX_NAV_PVT_ID  0x07
#define UBX_NAV_PVT_MAXSIZE  92
#define UBX_MAX_PAYLOAD 128

// UBX-NAV-PVT Payload structure in total 92 bytes
typedef struct __attribute__((packed)) {
    uint32_t iTOW;       // GPS time of week [ms]
    uint16_t year;       // Year (UTC)
    uint8_t  month;      // Month (UTC)
    uint8_t  day;        // Day (UTC)
    uint8_t  hour;       // Hour (UTC)
    uint8_t  min;        // Min (UTC)
    uint8_t  sec;        // Sec (UTC)
    uint8_t  valid;      // Validity flags
    uint32_t tAcc;       // Time accuracy estimate [ns]
    int32_t  nano;       // Fraction of second [ns]
    uint8_t  fixType;    // GNSSfix Type (0: no fix, 3: 3D fix... etc)
    uint8_t  flags;      // Fix status flags
    uint8_t  flags2;     // Additional flags
    uint8_t  numSV;      // Number of satellites used in Nav Solution
    int32_t  lon;        // Longitude [deg * 1e-7]
    int32_t  lat;        // Latitude [deg * 1e-7]
    int32_t  height;     // Height above ellipsoid [mm]
    int32_t  hMSL;       // Height above mean sea level [mm]
    uint32_t hAcc;       // Horizontal accuracy estimate [mm]
    uint32_t vAcc;       // Vertical accuracy estimate [mm]
    int32_t  velN;       // NED north velocity [mm/s]
    int32_t  velE;       // NED east velocity [mm/s]
    int32_t  velD;       // NED down velocity [mm/s]
    int32_t  gSpeed;     // Ground Speed (2D) [mm/s]
    int32_t  headMot;    // Heading of motion [deg * 1e-5]
    uint32_t sAcc;       // Speed accuracy estimate [mm/s]
    uint32_t headAcc;    // Heading accuracy estimate [deg * 1e-5]
    uint16_t pDOP;       // Position DOP [* 0.01]
    uint16_t flags3;     // Additional flags
    uint8_t  reserved1[4]; // Reserved
    int32_t  headVeh;    // Heading of vehicle [deg * 1e-5]
    int16_t  magDec;     // Magnetic declination [deg * 1e-2]
    uint16_t magAcc;     // Magnetic declination accuracy [deg * 1e-2]
} GpsData;

typedef enum {
    STATE_IDLE, 
    STATE_SYNC1, 
    STATE_SYNC2, 
    STATE_CLASS, 
    STATE_ID, 
    STATE_LEN_L, 
    STATE_LEN_H, 
    STATE_PAYLOAD, 
    STATE_CHKA, 
    STATE_CHKB
} ubx_state_t;

extern SemaphoreHandle_t gps_mutex;

void setGpsInterval(int new_interval);
int getGpsInterval(void);


uint8_t* get_gps_data_ptr(void);
int getSizeOfGpsVar();

GpsData getData();

void InitMux();


//method that parses the ubx bytes
void parse_ubx_byte(uint8_t byte);

void calculate_mock_checksum(uint8_t *data, size_t len, uint8_t *ck_a, uint8_t *ck_b);
#endif