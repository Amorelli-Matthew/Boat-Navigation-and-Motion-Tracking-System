#include "GPSDriver.h"

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

char *gprmcArray[14];
char *GPGGAArray[4];
char *GPVTGArray[3];
int indexbuffer = 0;

static GpsData CurrentGpsData = GPS_DATA_DEFAULTS;


static int gpsInterval = 2 * 1000 ; //ms
 
// Mode indicator
char modeInicatorList[] = { 'A', 'D', 'E', 'N', 'P' };
   

    float cog;
    float sog;
    char modeInicator;


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
    // 1. Generate realistic components
    int degLat = esp_random() % 90;
    double minLat = (esp_random() % 600000) / 10000.0;
    // NMEA Format: DDMM.MMMM (Degrees * 100 + Minutes)
    double nmeaLat = (degLat * 100.0) + minLat;

    int degLon = esp_random() % 180;
    double minLon = (esp_random() % 600000) / 10000.0;
    double nmeaLon = (degLon * 100.0) + minLon;

    int n = snprintf(buffer, (size_t)len,
        "$GPRMC,%02d%02d%02d,A,%09.4f,%c,%010.4f,%c,%.1f,%.1f,%02d%02d%02d,,",
        (int)(esp_random() % 24), (int)(esp_random() % 60), (int)(esp_random() % 60),
        nmeaLat, (esp_random() & 1 ? 'N' : 'S'),
        nmeaLon, (esp_random() & 1 ? 'E' : 'W'),
        (double)(esp_random() % 200 / 10.0), // Speed
        (double)(esp_random() % 3600 / 10.0), // Course
        (int)(1 + esp_random() % 12), (int)(1 + esp_random() % 28), (int)(esp_random() % 100)
    );

    if (n > 0 && n < len) {
        add_checksum(buffer, len);
    }
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
        (hours), minutes, seconds,
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
    cog = (esp_random() % 3600) / 10.0f;   // 0.0..359.9°
    sog = (esp_random() % 800) / 10.0f;    // 0.0..79.9 knots

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


 double degrees, minutes,rawLat, rawLon,decimalLat ;


void ParseGPRMCMessage(char* buffer, short len)
{

    indexbuffer = 0;
    char *token;

    // Skip the GPRMC identifier
    strsep(&buffer, ",");

    //Read through the identifier
    while ((token = strsep(&buffer, ",")) != NULL && indexbuffer < 13) {
        gprmcArray[indexbuffer] = token;
        indexbuffer++;
    }

    // Copy time string (first 6 characters)
        strncpy(CurrentGpsData.time, gprmcArray[0], 6);
        CurrentGpsData.time[6] = '\0';
    
    // Status
    CurrentGpsData.status = gprmcArray[1][0];
    

if (gprmcArray[2] != NULL) {
    rawLat = atof(gprmcArray[2]);
    degrees = floor(rawLat / 100.0);
    minutes = rawLat - (degrees * 100.0);
    
    //Convert to decimal
    decimalLat = degrees + (minutes / 60.0);
    
    //Apply Hemisphere
    if (gprmcArray[3] != NULL && gprmcArray[3][0] == 'S') {
        decimalLat *= -1.0;
    }

    // 3. Trap to 8 decimal places
    CurrentGpsData.latitude = round(decimalLat * 1e8) / 1e8;
}

if (gprmcArray[4] != NULL) {
    rawLon = atof(gprmcArray[4]);
    degrees = floor(rawLon / 100.0);
    minutes = rawLon - (degrees * 100.0);
    
    // 1. Convert to decimal
    double decimalLon = degrees + (minutes / 60.0);
    
    // 2. Apply Hemisphere
    if (gprmcArray[5] != NULL && gprmcArray[5][0] == 'W') {
        decimalLon *= -1.0;
    }

    // 3. Trap to 8 decimal places
    CurrentGpsData.longitude = round(decimalLon * 1e8) / 1e8;
}

    if(gprmcArray[3] != NULL)
     CurrentGpsData.lat_hemisphere = gprmcArray[3][0];

    if(gprmcArray[5] != NULL)
     CurrentGpsData.lon_hemisphere = gprmcArray[5][0];
        
    // Speed in knots
        CurrentGpsData.speed_knots = atof(gprmcArray[6]);
    
    // Course
        CurrentGpsData.course_true = atof(gprmcArray[7]);
    
    
    // Date
         strncpy(CurrentGpsData.date, gprmcArray[8], 6);
        CurrentGpsData.date[6] = '\0';
        

}

void ParseGPGGAMessage(char* buffer, short len)
{


}


 void ParsePVTGMessage(char* buffer, short len)
 {

 }
 