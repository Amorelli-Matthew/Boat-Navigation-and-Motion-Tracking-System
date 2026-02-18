package com.example.boattrackingsystem

import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Handler
import android.os.Looper
import com.welie.blessed.BluetoothCentralManager
import com.welie.blessed.BluetoothCentralManagerCallback
import com.welie.blessed.BluetoothPeripheral
import com.welie.blessed.BluetoothPeripheralCallback
import com.welie.blessed.GattStatus
import com.welie.blessed.HciStatus
import java.util.UUID

import  com.example.boattrackingsystem.BleUUIDS
import java.nio.ByteBuffer
import java.nio.ByteOrder

class BleManager(private val context: Context, val onStatusUpdate: (String) -> Unit) {

    private var lastRawData: String = ""
    private var activePeripheral: BluetoothPeripheral? = null // Store it here
    private var latestPacket: ByteArray  = byteArrayOf()

    var servicesReady = false

    val GPS_SERVICE_UUID: UUID = UUID.fromString(BleUUIDS.GPS_SERVICE_UUID_string)
    val GPS_DATA_CHAR_UUID: UUID = UUID.fromString(BleUUIDS.GPS_DATA_CHAR_UUID_string)
    //uuids of service
    private lateinit var central: BluetoothCentralManager



    // 3. Central callback (finding devices)
    private val centralCallback = object : BluetoothCentralManagerCallback() {
        // Note: The method name is 'onDiscovered' in the Kotlin version
        override fun onDiscovered(peripheral: BluetoothPeripheral, scanResult: ScanResult) {
            activePeripheral = peripheral
            central.stopScan()
            central.connect(peripheral, peripheralCallback)
        }
        override fun onConnectionFailed(peripheral: BluetoothPeripheral, status: HciStatus) {
            // Triggered if the connection attempt fails
        }

        override fun onDisconnected(peripheral: BluetoothPeripheral, status: HciStatus) {
            activePeripheral = null

            val message = when (status) {
                HciStatus.SUCCESS -> "Disconnected from ${peripheral.name}"
                HciStatus.CONNECTION_TIMEOUT -> "Lost connection (Timed out)"
                HciStatus.REMOTE_USER_TERMINATED_CONNECTION -> "ESP32 closed the connection"
                else -> "Disconnected: Error $status"
            }

            onStatusUpdate(message)
        }
        override fun onConnected(peripheral: BluetoothPeripheral) {

            if(activePeripheral != null) {
                servicesReady = true
                onStatusUpdate("Connected to ${peripheral.name}")
            }
            else
                onStatusUpdate("Already connected")
        }

    }

    private val peripheralCallback = object : BluetoothPeripheralCallback() {
        override fun onServicesDiscovered(peripheral: BluetoothPeripheral) {
            val hasChar = peripheral.services
                .flatMap { it.characteristics }
                .any { it.uuid == GPS_DATA_CHAR_UUID }

            if (!hasChar) {
                onStatusUpdate("Error: GPS Characteristic not found")
            }
        }

        override fun onCharacteristicUpdate(
            peripheral: BluetoothPeripheral,
            value: ByteArray,
            characteristic: BluetoothGattCharacteristic,
            status: GattStatus
        ) {
            // Explicitly defining the UUID type here fixes the "Recursive problem" error
            val charUuid: UUID = characteristic.uuid

            when(charUuid) {
                GPS_DATA_CHAR_UUID -> {
                    if (status == GattStatus.SUCCESS) {
                        ParseGPSSentence(value)
                    }
                }


            }


        }
    }




    init {
        // Initialize central here so 'centralCallback' can reference it later
        central = BluetoothCentralManager(context, centralCallback, Handler(Looper.getMainLooper()))
    }


    fun scanForSpecificDevice(DeviceName: String ) {
        central.stopScan()

        central.scanForPeripheralsWithNames(setOf(DeviceName))
    }

    fun disconnectDevice()
    {
        activePeripheral?.let {
            central.cancelConnection(it)
        } ?: run {
            onStatusUpdate("No device connected to disconnect")
        }
    }

    fun RecieveData()
    {
        if (!servicesReady)
            return

        activePeripheral?.readCharacteristic(GPS_SERVICE_UUID, GPS_DATA_CHAR_UUID)
    }

    /*
    Need to update function as some vars like latitude and Longitude are Doubles
    */
    fun ParseGPSSentence(stringOfBytes: ByteArray) {
        //Demo Mode
//        if (!servicesReady)
//            return
        var time = CharArray(6) { i -> stringOfBytes.copyOfRange(0, 6)[i].toInt().toChar() }
        var status = stringOfBytes[7].toInt().toChar()
        var latitude = byteArrayToFloat(stringOfBytes.copyOfRange(8, 12))
        var latHemisphere = stringOfBytes[12].toInt().toChar()
        var longitude = byteArrayToFloat(stringOfBytes.copyOfRange(13, 17))
        var long_hemisphere = stringOfBytes[17].toInt().toChar()

        var speed = byteArrayToFloat(stringOfBytes.copyOfRange(18, 22))
        var courseTrue = byteArrayToFloat(stringOfBytes.copyOfRange(22, 26))

        var date = CharArray(6) { i -> stringOfBytes.copyOfRange(26, 32)[i].toInt().toChar() }

        var altitude = byteArrayToFloat(stringOfBytes.copyOfRange(33, 37))
        var altitudeUnit = stringOfBytes[37].toInt().toChar()
//
        var hdop = byteArrayToFloat(stringOfBytes.copyOfRange(38, 42))

       var numSatellites = byteArrayToInt(stringOfBytes.copyOfRange(42, 46))

      var fixQuality = stringOfBytes[46].toInt().toChar()
        var geoidHeight = byteArrayToFloat(stringOfBytes.copyOfRange(47, 51))

        var geoidUnit = stringOfBytes[51].toInt().toChar()
       var ageDgps = byteArrayToFloat(stringOfBytes.copyOfRange(52, 56))
        var dgpsId = byteArrayToInt(stringOfBytes.copyOfRange(56, 60))
        var speedKmh = byteArrayToFloat(stringOfBytes.copyOfRange(60, 64))
        var courseMagnetic = byteArrayToFloat(stringOfBytes.copyOfRange(64, 68))
        var modeIndicator = stringOfBytes[68].toInt().toChar()

        onStatusUpdate(
            """Raw data: ${stringOfBytes.toHexString( HexFormat {
                bytes {
                    byteSeparator = " "
                    upperCase = true
                }
            })}
    Time: ${time.concatToString()} | Date: ${date.concatToString()}
    Status: $status | Fix: $fixQuality
    Pos: $latitude $latHemisphere, $longitude $long_hemisphere
    Speed: $speed knots ($speedKmh km/h)
    Course: $courseTrue (Mag: $courseMagnetic)
    Alt: $altitude $altitudeUnit | | Geoid: $geoidHeight $geoidUnit    Sats: $numSatellites | HDOP: $hdop 
    Age: $ageDgps | DgpsId: $dgpsId  | Mode: $modeIndicator
""".trimIndent()
        )
    }

    //helper for converting bytes to floats
    fun byteArrayToDouble(bytes: ByteArray): Double {
        // Ensure the byte array has exactly 4 bytes (size of a Float)
        require(bytes.size == 4) { "Byte array must have a size of 4 to convert to a single Float" }

        // Wrap the byte array in a ByteBuffer
        val buffer = ByteBuffer.wrap(bytes)

        // Specify the byte order
        //little-endian
        buffer.order(ByteOrder.LITTLE_ENDIAN)

        // Get the float value
        return buffer.getDouble()
    }

    //helper for converting bytes to floats
    fun byteArrayToFloat(bytes: ByteArray): Float {
        // Ensure the byte array has exactly 4 bytes (size of a Float)
        require(bytes.size == 4) { "Byte array must have a size of 4 to convert to a single Float" }

        // Wrap the byte array in a ByteBuffer
        val buffer = ByteBuffer.wrap(bytes)

        // Specify the byte order
        //little-endian
        buffer.order(ByteOrder.LITTLE_ENDIAN)

        // Get the float value
        return buffer.getFloat()
    }
    fun byteArrayToInt(bytes: ByteArray): Int {
        // Ensure the byte array has exactly 4 bytes (size of a Float)
        require(bytes.size == 4) { "Byte array must have a size of 4 to convert to a single Float" }

        // Wrap the byte array in a ByteBuffer
        val buffer = ByteBuffer.wrap(bytes)

        // Specify the byte order
        //little-endian
        buffer.order(ByteOrder.LITTLE_ENDIAN)

       // Get the float value
        return buffer.getInt()
    }



    }



