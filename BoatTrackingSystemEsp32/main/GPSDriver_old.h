#ifndef GPS_Driver_H
#define GPS_Driver_H

#include "esp_random.h"
#include <math.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"


#define GPS_DATA_DEFAULTS { \
     \
    .latitude = 0.0, \
    .longitude = 0.0, \
    \
    \
    .altitude = 0.0f, \
    .speed_knots = 0.0f, \
    .speed_kmh = 0.0f, \
    .course_true = 0.0f, \
    .course_magnetic = 10.0f, \
    .hdop = 99.9f, \
    .geoid_height = 10.0f, \
    .age_dgps = 0.0f, \
    .num_satellites = 0, \
    .dgps_station_id = 0, \
    \
     \
    .time = "000000", \
    .date = "010120", \
    .status = 'V', \
    .lat_hemisphere = 'N', \
    .lon_hemisphere = 'E', \
    .altitude_unit = 'M', \
    .fix_quality = '0', \
    .geoid_unit = 'M', \
    .mode_indicator = 'N' \
}


//struct to stored parsed gps data
//reordering data strctrures so that everything is grouped together by name helps avoide attribute packed
//typedef struct{ 
typedef struct __attribute__((packed)) { 
    
    //GPRMC
    char time[7];          // hhmmss\0
    char status;           // A or V
    double latitude;       // High precision (8 bytes)
    char lat_hemisphere;   // N or S
    double longitude;      // High precision (8 bytes)
    char lon_hemisphere;   // E or W
    float speed_knots;     // knots
    float course_true;     // degrees
    char date[7];          // ddmmyy\0

    // GPGGA specific fields
    float altitude;        // meters
    char altitude_unit;    // 'M'
    float hdop;            // Horizontal Dilution
    int num_satellites;    // Number of satellites
    char fix_quality;      // Fix quality flag
    float geoid_height;    // Geoid separation
    char geoid_unit;       // 'M'
    float age_dgps;        // Seconds since update
    int dgps_station_id;   // Station ID
    
    // GPVTG specific fields  
    float speed_kmh;       // Speed in km/h
    float course_magnetic; // Magnetic course
    char mode_indicator;   // NMEA mode flag
} GpsData;




extern SemaphoreHandle_t gps_mutex;


//the generate random Nema sentences will be removed



void setGpsInterval(int new_interval);
int getGpsInterval(void);


uint8_t* get_gps_data_ptr(void);
int getSizeOfGpsVar();

GpsData getData();


void InitMux();

void generateRandomGPRMC(char* buffer, short len);
void ParseGPRMCMessage(char* buffer, short);

void generateRandomGPGGA(char* buffer, short len);
void ParseGPGGAMessage(char* buffer, short len);

void generateRandomGPVTG(char* buffer, short len);
void ParseGPVTGMessage(char* buffer, short len);
 

uint8_t nmea_checksum_payload(const char *payload);
void add_checksum(char *out, size_t outlen);

#endif