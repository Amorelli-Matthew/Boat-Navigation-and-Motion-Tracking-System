<<<<<<< HEAD:BoatTrackingSystemEsp32/main/GPSDriver.h
#ifndef GPS_Driver_H
#define GPS_Driver_H

#include "esp_random.h"
#include <math.h>
#include <stdbool.h>

#define GPS_DATA_DEFAULTS { \
    .time = "000000", \
    .status = 'V', \
    .latitude = 0.0f, \
    .lat_hemisphere = 'N', \
    .longitude = 0.0f, \
    .lon_hemisphere = 'E', \
    .speed_knots = 0.0f, \
    .course_true = 0.0f, \
    .date = "010120", \
    .altitude = 0.0f, \
    .altitude_unit = 'M', \
    .hdop = 99.9f, \
    .num_satellites = 0, \
    .fix_quality = '0', \
    .geoid_height = 10.0f, \
    .geoid_unit = 'M', \
    .age_dgps = 0.0f, \
    .dgps_station_id = 0, \
    .speed_kmh = 0.0f, \
    .course_magnetic = 10.0f, \
    .mode_indicator = 'N' \
}

//struct to stored parsed gps data
typedef struct __attribute__((packed)){ 
//typedef struct {
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


//the generate random Nema sentences will be removed


uint8_t* get_gps_data_ptr(void);
unsigned long getSizeOfGpsVar();

void generateRandomGPRMC(char* buffer, short len);
void ParseGPRMCMessage(char* buffer, short);

void generateRandomGPGGA(char* buffer, short len);
void ParseGPGGAMessage(char* buffer, short len);

void generateRandomGPVTG(char* buffer, short len);
void ParseGPVTGMessage(char* buffer, short len);
 

uint8_t nmea_checksum_payload(const char *payload);
void add_checksum(char *out, size_t outlen);

=======
#ifndef GPS_Driver_H
#define GPS_Driver_H

#include "esp_random.h"

void generateRandomGPRMC(char* buffer, short len);
void ParseGPRMCMessage(char* buffer);

void generateRandomGPGGA(char* buffer, short len);
void ParseGPGGAMessage(char* buffer, short len);

 void generateRandomGPVTG(char* buffer, short len);
 void ParseGPVTGMessage(char* buffer, short len);
 

uint8_t nmea_checksum_payload(const char *payload);
 void add_checksum(char *out, size_t outlen);

>>>>>>> parent of 520adba (started bluetooth implimentation and parsing gps sentences):main/GPSDriver.h
#endif