<<<<<<< HEAD:BoatTrackingSystemEsp32/main/GPSDriver.c
#include "GPSDriver.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

char *gprmcArray[13];

int indexbuffer = 0;

static GpsData CurrentGpsData = GPS_DATA_DEFAULTS;


    float cog;
    float sog;
    char modeInicator;

     char modeInicatorList[] = { 'A', 'D', 'E', 'N', 'P'};

uint8_t* get_gps_data_ptr(void) {
    return (uint8_t*)&CurrentGpsData;
}
unsigned long getSizeOfGpsVar()
{
    return sizeof(CurrentGpsData);
}


uint8_t nmea_checksum_payload(const char* payload)
{
    uint8_t cs = 0;
    if (!payload) return 0;

    // Skip leading '$' if present
    if (*payload == '$') payload++;

    for (const unsigned char *p = (const unsigned char*)payload; *p; ++p) {
        if (*p == '*' || *p == '\r' || *p == '\n') break;
        cs ^= *p;
    }
    return cs;
}

void add_checksum(char *out, size_t outlen) {
    if (!out || outlen < 5) return;

    uint8_t cs = nmea_checksum_payload(out);

    size_t used = strnlen(out, outlen);
    if (used + 5 < outlen) {
        // Append checksum and CRLF
        snprintf(out + used, outlen - used, "*%02X\r\n", cs);
    } else {
        // Not enough space; just terminate safely
        if (outlen > 0) out[outlen - 1] = '\0';
    }
}


void generateRandomGPRMC(char* buffer, short len) {
    

    memset(buffer, 0, (size_t)len);

    //Builds a test buffer with algs to calc the fields
    
    int n = snprintf(buffer, (size_t)len,
        "$GPRMC,%02d%02d%02d,A,%02d%07.4f,%c,%03d%07.4f,%c,%.1f,%.1f,%02d%02d%02d,,",
        (int)( esp_random() % 24), (int)(esp_random() % 60), (int)(esp_random() % 60),
        (int)(esp_random() % 90), (double)((esp_random() % 600000) / 10000.0f), (int) ((esp_random() & 1) ? 'N' : 'S'),
        (int)(esp_random() % 180), (double)((esp_random() % 600000) / 10000.0f),  ((esp_random() & 1) ? 'N' : 'S'),
        (double)((esp_random() % 2000) / 10.0f),
        (double)((esp_random() % 3600) / 10.0f),
       (int) ((esp_random() % 12)), (int)(1 + (esp_random() % 28)), (int)(esp_random() % 100)
    );

    if (n < 0 || n >= len) {
        // Truncated or error; ensure termination
        if (len > 0) buffer[len - 1] = '\0';
        return;
    }

    add_checksum(buffer, len);
}

void generateRandomGPGGA(char* buffer, short len)

{
    if (!buffer || len <= 32) return; 

    //Time (UTC)
    int hours   = esp_random() % 24;
    int minutes = esp_random() % 60;
    int seconds = esp_random() % 60;

    //Latitude
    int   latDegrees    = esp_random() % 90;                  
    float latMinutes    = (esp_random() % 600000) / 10000.0f;  
    char  latHemisphere = (esp_random() & 1) ? 'N' : 'S';

    //Longitude
    int   lonDegrees    = esp_random() % 180;                 
    float lonMinutes    = (esp_random() % 600000) / 10000.0f;  
    char  lonHemisphere = (esp_random() & 1) ? 'E' : 'W';

    //Fix data
    int   fixQuality    = (esp_random() % 10 < 8) ? 1 : 2;    
    int   satellitesUsed= 4 + (esp_random() % 9);              
    float hdop          = 5 + (esp_random() % 21);             
    hdop /= 10.0f;                                           

    float altitude      = 5 + (esp_random() % 195);            
    float geoidSep      = 10 + (esp_random() % 30);           
    
    // If degrees hit the max, force minutes to 0.0000

    if (latDegrees == 89 && latMinutes > 59.9999f) latMinutes = 59.9999f;
    if (lonDegrees == 179 && lonMinutes > 59.9999f) lonMinutes = 59.9999f;

    memset(buffer, 0, (size_t)len);

    // $GPGGA
    int n = snprintf(buffer, (size_t)len,
        "$GPGGA,%02d%02d%02d,%02d%07.4f,%c,%03d%07.4f,%c,%d,%02d,%.1f,%.1f,M,%.1f,M,,",
        hours, minutes, seconds,
        latDegrees, latMinutes,  latHemisphere,
        lonDegrees, lonMinutes,  lonHemisphere,
        fixQuality,
        satellitesUsed,
        hdop,
        altitude,
        geoidSep
    );

    if (n < 0 || n >= len) { if (len > 0) buffer[len - 1] = '\0'; return; }

        add_checksum(buffer, (size_t)len);
}
    

 void generateRandomGPVTG(char* buffer, short len)
 {
    // Random course & speed
    float cog = (esp_random() % 3600) / 10.0f;   // 0.0..359.9°
    float sog = (esp_random() % 800) / 10.0f;    // 0.0..79.9 knots

    // Mode indicator
    char modeInicatorList[] = { 'A', 'D', 'E', 'N', 'P' };
    char modeIndicator = modeInicatorList[esp_random() % 5];

    // Convert knots to km/h
    float sog_kmh = sog * 1.852f;

    memset(buffer, 0, (size_t)len);

    // Build payload
    int n = snprintf(buffer, (size_t)len,
        "$GPVTG,%.1f,T,,M,%.1f,N,%.1f,K,%c",
        cog,
        sog,
        sog_kmh,
        modeIndicator
    );

    if (n < 0 || n >= len) { if (len > 0) buffer[len - 1] = '\0'; return; }

    // Append checksum + CRLF
    add_checksum(buffer, (size_t)len);
  
 }




void ParseGPRMCMessage(char* buffer, short len)
{
    indexbuffer = 0;
    char *token;

    // Skip the GPRMC identifier
    strsep(&buffer, ",");

    //Read through the identifier
    while ((token = strsep(&buffer, ",")) != NULL && indexbuffer < 13) {
        gprmcArray[indexbuffer] = token;
        printf("Token %d value: %s\n", indexbuffer, gprmcArray[indexbuffer]);
        indexbuffer++;
    }

    // Copy time string (first 6 characters)
        strncpy(CurrentGpsData.time, gprmcArray[0], 6);
        CurrentGpsData.time[6] = '\0';
    
    // Status
    CurrentGpsData.status = gprmcArray[1][0];
    
    
    // Latitude, needs conversion from DDMM.MMMM to decimal degrees
    if (gprmcArray[2] != NULL) {
        CurrentGpsData.latitude = atof(gprmcArray[2]);

        // Convert from DDMM.MMMM to decimal degrees
        float degrees = floor(CurrentGpsData.latitude / 100);
        
        float minutes = CurrentGpsData.latitude - (degrees * 100);

        CurrentGpsData.latitude = degrees + (minutes / 60.0f);
        
        // Apply hemisphere
        if (gprmcArray[3][0] == 'S') {
            CurrentGpsData.latitude = -CurrentGpsData.latitude;
        }
    }
    

    // Longitude, needs conversion from DDDMM.MMMM to decimal degrees
    
        CurrentGpsData.longitude = atof(gprmcArray[4]);
        // Convert from DDDMM.MMMM to decimal degrees
        float degrees = floor(CurrentGpsData.longitude / 100);
        
        float minutes = CurrentGpsData.longitude - (degrees * 100);

        CurrentGpsData.longitude = degrees + (minutes / 60.0f);
        
        // Apply hemisphere
        if (gprmcArray[5][0] == 'W') {
            CurrentGpsData.longitude = -CurrentGpsData.longitude;
        }
        
    // Speed in knots
        CurrentGpsData.speed_knots = atof(gprmcArray[6]);
    
    // Course
        CurrentGpsData.course_true = atof(gprmcArray[7]);
    
    
    // Date
         strncpy(CurrentGpsData.date, gprmcArray[8], 6);
        CurrentGpsData.date[6] = '\0';
        
    // Valid flag based on status
    //CurrentGpsData.valid = (CurrentGpsData.status == 'A');
    
    // Debug
    // printf("Date: %s\n", CurrentGpsData.date);
    // printf("Latitude: %f\n", CurrentGpsData.latitude);
    // printf("Longitude: %f\n", CurrentGpsData.longitude);
//    printf("Size of GpsData: %zu bytes\n", sizeof(int));

}

void ParseGPGGAMessage(char* buffer, short len)
{

}


 void ParsePVTGMessage(char* buffer, short len)
 {

 }
=======
#include "GPSDriver.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>


//might convert to struct

 // Generate random time (hhmmss)
     short hours;
     short minutes;
     short seconds; 

    // Generate random latitude (ddmm.mmmm,N/S)
     short latDegrees;
     float latMinutes;
     char latHemisphere; 

    // Generate random longitude (dddmm.mmmm,E/W)
     short lonDegrees;
     float lonMinutes; 
     char lonHemisphere; 

    // Generate random speed and track angle
    float speed; 
    float trackAngle; 

    // Generate random date (ddmmyy)
     short day; 
     short month;
     short year;

short fixQuality;
short satellitesUsed;
float hdop;
float altitude;
float hight;

#include <stdint.h>
#include <stdio.h>
#include <string.h>

uint8_t nmea_checksum_payload(const char* payload)
{
    uint8_t cs = 0;
    if (!payload) return 0;

    // Skip leading '$' if present
    if (*payload == '$') payload++;

    for (const unsigned char *p = (const unsigned char*)payload; *p; ++p) {
        if (*p == '*' || *p == '\r' || *p == '\n') break;
        cs ^= *p;
    }
    return cs;
}

void add_checksum(char *out, size_t outlen) {
    if (!out || outlen < 5) return;

    uint8_t cs = nmea_checksum_payload(out);

    size_t used = strnlen(out, outlen);
    if (used + 5 < outlen) {
        // Append checksum and CRLF
        snprintf(out + used, outlen - used, "*%02X\r\n", cs);
    } else {
        // Not enough space; just terminate safely
        if (outlen > 0) out[outlen - 1] = '\0';
    }
}


void generateRandomGPRMC(char* buffer, short len) {

    // Time
    int hours   = esp_random() % 24;        // 0..23
    int minutes = esp_random() % 60;        // 0..59
    int seconds = esp_random() % 60;        // 0..59

    // Latitude (avoid 90° with nonzero minutes)
    int   latDegrees    = esp_random() % 90;                        
    float latMinutes    = (esp_random() % 600000) / 10000.0f;      
    char  latHemisphere = (esp_random() & 1) ? 'N' : 'S';

    // Longitude (avoid 180° with nonzero minutes)
    int   lonDegrees    = esp_random() % 180;                        
    float lonMinutes    = (esp_random() % 600000) / 10000.0f;      
    char  lonHemisphere = (esp_random() & 1) ? 'E' : 'W';

    // Speed (knots) and course (deg)
    float speed      = (esp_random() % 2000) / 10.0f;                
    float trackAngle = (esp_random() % 3600) / 10.0f;                

    // Date (use always-valid day 1..28)
    int month = 1 + (esp_random() % 12);                             // 1..12
    int day   = 1 + (esp_random() % 28);                        
    int year  = esp_random() % 100;                               

    memset(buffer, 0, (size_t)len);

    
    int n = snprintf(buffer, (size_t)len,
        "$GPRMC,%02d%02d%02d,A,%02d%07.4f,%c,%03d%07.4f,%c,%.1f,%.1f,%02d%02d%02d,,",
        hours, minutes, seconds,
        latDegrees, latMinutes,  latHemisphere,
        lonDegrees, lonMinutes,  lonHemisphere,
        speed,
        trackAngle,
        day, month, year
    );

    if (n < 0 || n >= len) {
        // Truncated or error; ensure termination
        if (len > 0) buffer[len - 1] = '\0';
        return;
    }

    add_checksum(buffer, len);
}

void generateRandomGPGGA(char* buffer, short len)

{
    if (!buffer || len <= 32) return; 

    // --- Time (UTC) ---
    int hours   = esp_random() % 24;
    int minutes = esp_random() % 60;
    int seconds = esp_random() % 60;

    // --- Latitude ---
    int   latDegrees    = esp_random() % 90;                  
    float latMinutes    = (esp_random() % 600000) / 10000.0f;  
    char  latHemisphere = (esp_random() & 1) ? 'N' : 'S';

    // --- Longitude ---
    int   lonDegrees    = esp_random() % 180;                 
    float lonMinutes    = (esp_random() % 600000) / 10000.0f;  
    char  lonHemisphere = (esp_random() & 1) ? 'E' : 'W';

    // --- Fix data ---
    // Fix quality: 0=invalid,1=GPS,2=DGPS,4=RTK fixed,5=RTK float, etc.
    int   fixQuality    = (esp_random() % 10 < 8) ? 1 : 2;    
    int   satellitesUsed= 4 + (esp_random() % 9);              
    float hdop          = 5 + (esp_random() % 21);             
    hdop /= 10.0f;                                           

    float altitude      = 5 + (esp_random() % 195);            
    float geoidSep      = 10 + (esp_random() % 30);           
    // If degrees hit the max, force minutes to 0.0000 (to avoid 90°xx' or 180°xx')
    if (latDegrees == 89 && latMinutes > 59.9999f) latMinutes = 59.9999f;
    if (lonDegrees == 179 && lonMinutes > 59.9999f) lonMinutes = 59.9999f;

    memset(buffer, 0, (size_t)len);

    // $GPGGA,<hhmmss>,<lat>,<N/S>,<lon>,<E/W>,<fix>,<sats>,<HDOP>,<alt>,M,<geoid>,M,<diff age>,<diff station>
    int n = snprintf(buffer, (size_t)len,
        "$GPGGA,%02d%02d%02d,%02d%07.4f,%c,%03d%07.4f,%c,%d,%02d,%.1f,%.1f,M,%.1f,M,,",
        hours, minutes, seconds,
        latDegrees, latMinutes,  latHemisphere,
        lonDegrees, lonMinutes,  lonHemisphere,
        fixQuality,
        satellitesUsed,
        hdop,
        altitude,
        geoidSep
    );

    if (n < 0 || n >= len) { if (len > 0) buffer[len - 1] = '\0'; return; }

        add_checksum(buffer, (size_t)len);
}

    float cog;
    float sog;
    char modeInicator;

     char modeInicatorList[] = { 'A', 'D', 'E', 'N', 'P'};
    

 void generateRandomGPVTG(char* buffer, short len)
 {
    // Random course & speed
    float cog = (esp_random() % 3600) / 10.0f;   // 0.0..359.9°
    float sog = (esp_random() % 800) / 10.0f;    // 0.0..79.9 knots

    // Mode indicator
    char modeInicatorList[] = { 'A', 'D', 'E', 'N', 'P' };
    char modeIndicator = modeInicatorList[esp_random() % 5];

    // Convert knots to km/h
    float sog_kmh = sog * 1.852f;

    memset(buffer, 0, (size_t)len);

    // Build payload
    int n = snprintf(buffer, (size_t)len,
        "$GPVTG,%.1f,T,,M,%.1f,N,%.1f,K,%c",
        cog,
        sog,
        sog_kmh,
        modeIndicator
    );

    if (n < 0 || n >= len) { if (len > 0) buffer[len - 1] = '\0'; return; }

    // Append checksum + CRLF
    add_checksum(buffer, (size_t)len);
  
 }

void ParseGPGGAMessage(char* buffer, short len)
{

}


 void ParsePVTGMessage(char* buffer, short len)
 {

 }
>>>>>>> parent of 520adba (started bluetooth implimentation and parsing gps sentences):main/GPSDriver.c
 