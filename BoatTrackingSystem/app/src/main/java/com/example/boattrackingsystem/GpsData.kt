package com.example.boattrackingsystem

data class GpsData(
    //GPRMC data fields

    //Should convert to date object
    val time:String = "", //CharArray(6) { '0' },
    val status: Char = '0',
    val latitude: Double = 0.0,
    val latHemisphere: Char = '0',
    val longitude: Double = 0.0,
    val longHemisphere: Char = '0',
    val speedKnots: Float = 0.0f,
    val courseDegrees: Float = 0.0f,
    //Should convert to date object
    val Date: String = "", // CharArray = CharArray(6) { '0' },

    // GPGGA specific fields
    val altitude: Float = 0.0f,
    val altitudeUnit: Char = 'M',
    val hdop: Float = 0.0f,
    val numOfSatellites: Int = 0,
    val fixQuality: Char = '0',
    val geoidHeight: Float = 0.0f,
    val geoidUnit: Char = 'M',
    val ageDgps: Float = 0.0f,
    val dgpsId: Int = 0,

    val speedKmh: Float = 0.0f,
    val courseMagnetic: Float = 0.0f,
    val modeIndicator: Char = '0'
) {
    override fun toString(): String {
    return    """Time: $time | Date: $Date
Status: $status | Fix: $fixQuality
Pos: $latitude $latHemisphere, $longitude $longHemisphere
Speed: $speedKnots knots ($speedKmh km/h)
Course: $courseDegrees (Mag: $courseMagnetic
Alt: $altitude $altitudeUnit | | Geoid: $geoidHeight $geoidUnit    Sats: $numOfSatellites | HDOP: $hdop
Age: $ageDgps | DgpsId: $dgpsId  | Mode: $modeIndicator"""
    }

}