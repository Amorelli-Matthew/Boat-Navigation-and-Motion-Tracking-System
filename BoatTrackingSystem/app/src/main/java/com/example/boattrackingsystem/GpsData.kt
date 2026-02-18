package com.example.boattrackingsystem

data class GpsData(
    //GPRMC data fields

    //Should convert to date object
    var time: CharArray = CharArray(6) { '0' },
    var status: Char = '0',
    var latitude: Double = 0.0,
    var lat_hemisphere: Char = '0',
    var longitude: Double = 0.0,
    var long_hemisphere: Char = '0',
    var speedKnots: Float = 0.0f,
    var courseDegrees: Float = 0.0f,
    //Should convert to date object
    var Date: CharArray = CharArray(6) { '0' },

    // GPGGA specific fields
    var altitude: Float = 0.0f,
    var altitudeUnit: Char = 'M',
    var hdop: Float = 0.0f,
    var numOfSatellites: Int = 0,
    var fixQuality: Char = '0',
    var geoidHeight: Float = 0.0f,
    var geoidUnit: Char = 'M',
    var ageDgps: Float = 0.0f,
    var dgpsStationId: Int = 0,

    var speedKmh: Float = 0.0f,
    var courseMagnetic: Float = 0.0f,
    var modeIndicator: Char = '0'
)
