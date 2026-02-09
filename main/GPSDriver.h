#ifndef GPS_Driver_H
#define GPS_Driver_H

#include "esp_random.h"
#include <math.h>
#include <stdbool.h>

//struct to stored parsed gprmc data
typedef struct __attribute__((packed)) {
    char time[7];          // hhmmss.ss
    char status;            // A or V
    float latitude;         // Decimal degrees
    char lat_hemisphere;    // N or S
    float longitude;        // Decimal degrees  
    char lon_hemisphere;    // E or W
    float speed_knots;      // knots
    float course_true;      // degrees
    char date[7];           // ddmmyy

        // GPGGA specific fields
    float altitude;        // meters above mean sea level
    char altitude_unit;    // 'M' for meters
    float hdop;            // Horizontal Dilution of Precision
    int num_satellites;    // Number of satellites in use
    char fix_quality;      // 0=no fix, 1=GPS fix, 2=DGPS fix, etc.
    float geoid_height;    // Geoid separation
    char geoid_unit;       // 'M' for meters
    float age_dgps;        // Seconds since last DGPS update
    int dgps_station_id;   // DGPS station ID
    
    // GPVTG specific fields  
    float speed_kmh;       // Speed in km/h
    float course_magnetic; // Magnetic course (if available)
    char mode_indicator;   // NMEA 2.3+ mode indicator
} GpsData;

// //needed for bluetooth
// typedef union __attribute__((packed)) {
//      GpsDataStruct gpsStruct;
//      uint8_t bytes[sizeof(GpsDataStruct)];
// } GpsData;

//the generate random Nema sentences will be removed


uint8_t* get_gps_data_ptr(void);

void generateRandomGPRMC(char* buffer, short len);
void ParseGPRMCMessage(char* buffer, short);

void generateRandomGPGGA(char* buffer, short len);
void ParseGPGGAMessage(char* buffer, short len);

void generateRandomGPVTG(char* buffer, short len);
void ParseGPVTGMessage(char* buffer, short len);
 

uint8_t nmea_checksum_payload(const char *payload);
void add_checksum(char *out, size_t outlen);

#endif