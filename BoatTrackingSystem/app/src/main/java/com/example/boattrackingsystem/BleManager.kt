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

import  com.example.myapplication.BleUUIDS
import java.nio.ByteBuffer

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

    fun ParseGPSSentence(stringOfBytes: ByteArray) {
        if (!servicesReady)
            return
        //val time: ByteArray = stringOfBytes.copyOfRange(0, 6)

        var time: CharArray = CharArray(6) { i -> stringOfBytes.copyOfRange(0, 6)[i].toInt().toChar() }

        var status = stringOfBytes[7].toInt().toChar()
        var latitude: Float = byteArrayToFloat(stringOfBytes.copyOfRange(8, 12))

        onStatusUpdate("Time is: ${time.concatToString()}\nstatus:${status}\nLatitude: ${latitude}");


    }
    //helper for converting bytes to floats
    fun byteArrayToFloat(bytes: ByteArray): Float {
        // Ensure the byte array has exactly 4 bytes (size of a Float)
        require(bytes.size == 4) { "Byte array must have a size of 4 to convert to a single Float" }

        // Wrap the byte array in a ByteBuffer
        val buffer = ByteBuffer.wrap(bytes)

        // Specify the byte order if necessary (e.g., if the data is little-endian)
        // Most Android/iOS devices are little-endian, but data from other sources might be big-endian
        // buffer.order(ByteOrder.LITTLE_ENDIAN)

        // Get the float value
        return buffer.getFloat()
    }


    }



