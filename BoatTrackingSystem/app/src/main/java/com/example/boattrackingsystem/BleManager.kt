package com.example.boattrackingsystem

import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.le.ScanResult
import android.content.Context
import android.os.Handler
import android.os.Looper
import androidx.annotation.Nullable
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
    fun ParseGPSSentence(stringOfBytes: ByteArray) : GpsData? {
        //Demo Mode
//        if (!servicesReady)
//            return
        val buffer = ByteBuffer.wrap(stringOfBytes).order(ByteOrder.LITTLE_ENDIAN)
        try {

            //create new Temporary GpsObject
            val currentGpsInterval:GpsData = GpsData(

             time = String(stringOfBytes, 0, 6, Charsets.US_ASCII),
            status = stringOfBytes[7].toInt().toChar(),

             latitude = buffer.getDouble(8),

             latHemisphere = stringOfBytes[16].toInt().toChar(),

            longitude = buffer.getDouble(17),
            longHemisphere = stringOfBytes[25].toInt().toChar(),

                    speedKnots = buffer.getFloat(26),
            courseDegrees = buffer.getFloat(30),

            Date = String(stringOfBytes, 34, 6, Charsets.US_ASCII),

         altitude = buffer.getFloat(41),
        altitudeUnit = stringOfBytes[45].toInt().toChar(),

        hdop = buffer.getFloat(46),

      numOfSatellites = buffer.getInt(50),

      fixQuality = stringOfBytes[54].toInt().toChar(),
      geoidHeight = buffer.getFloat(55),

       geoidUnit = stringOfBytes[59].toInt().toChar(),
       ageDgps = buffer.getFloat(60),

       dgpsId = buffer.getInt(64),
        speedKmh = buffer.getFloat(68),
        courseMagnetic = buffer.getFloat(72),
        modeIndicator = stringOfBytes[76].toInt().toChar()
            );



            return currentGpsInterval;
        }
        catch (e: Exception)
        {
            onStatusUpdate(e.toString())
        }


            return null;

    }

    }



