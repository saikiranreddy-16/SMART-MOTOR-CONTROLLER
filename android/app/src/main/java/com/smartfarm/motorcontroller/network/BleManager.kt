package com.smartfarm.motorcontroller.network

import android.annotation.SuppressLint
import android.bluetooth.*
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanResult
import android.content.Context
import android.util.Log
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import java.util.*

@SuppressLint("MissingPermission")
class BleManager(private val context: Context) {

    private val bluetoothAdapter: BluetoothAdapter? by lazy {
        val bluetoothManager = context.getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    private var bluetoothGatt: BluetoothGatt? = null
    private var telemetryCharacteristic: BluetoothGattCharacteristic? = null
    private var controlCharacteristic: BluetoothGattCharacteristic? = null

    private val _connectionState = MutableStateFlow("DISCONNECTED")
    val connectionState: StateFlow<String> = _connectionState

    private val _telemetryData = MutableStateFlow<String?>(null)
    val telemetryData: StateFlow<String?> = _telemetryData

    companion object {
        val SERVICE_UUID: UUID = UUID.fromString("4fafc201-1fb5-459e-8fcc-c5c9c331914b")
        val CHAR_TELEMETRY: UUID = UUID.fromString("beb5483e-36e1-4688-b7f5-ea07361b26a8")
        val CHAR_CONTROL: UUID = UUID.fromString("cba1a95e-18e1-4c2f-b2b9-e137351b26a9")
        const val TAG = "BleManager"
    }

    fun startScanning(onDeviceFound: (BluetoothDevice) -> Unit) {
        val scanner = bluetoothAdapter?.bluetoothLeScanner ?: return
        Log.d(TAG, "Starting BLE scan...")
        scanner.startScan(object : ScanCallback() {
            override fun onScanResult(callbackType: Int, result: ScanResult?) {
                result?.device?.let { device ->
                    if (device.name != null && device.name.startsWith("SmartPump_")) {
                        onDeviceFound(device)
                    }
                }
            }
        })
    }

    fun connect(device: BluetoothDevice) {
        _connectionState.value = "CONNECTING"
        bluetoothGatt = device.connectGatt(context, false, gattCallback)
    }

    fun disconnect() {
        bluetoothGatt?.disconnect()
        bluetoothGatt = null
        _connectionState.value = "DISCONNECTED"
    }

    fun sendControlCommand(command: String) {
        val characteristic = controlCharacteristic ?: return
        characteristic.value = command.toByteArray(Charsets.UTF_8)
        characteristic.writeType = BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
        bluetoothGatt?.writeCharacteristic(characteristic)
        Log.d(TAG, "Sent BLE command: $command")
    }

    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            if (newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "Connected to GATT server. Starting service discovery...")
                _connectionState.value = "CONNECTED"
                gatt?.discoverServices()
            } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                Log.d(TAG, "Disconnected from GATT server.")
                _connectionState.value = "DISCONNECTED"
                _telemetryData.value = null
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            if (status == BluetoothGatt.GATT_SUCCESS) {
                val service = gatt?.getService(SERVICE_UUID)
                telemetryCharacteristic = service?.getCharacteristic(CHAR_TELEMETRY)
                controlCharacteristic = service?.getCharacteristic(CHAR_CONTROL)

                // Enable notifications for telemetry
                telemetryCharacteristic?.let { char ->
                    gatt.setCharacteristicNotification(char, true)
                    val descriptor = char.getDescriptor(UUID.fromString("00002902-0000-1000-8000-00805f9b34fb"))
                    descriptor.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                    gatt.writeDescriptor(descriptor)
                    Log.d(TAG, "Subscribed to Telemetry notifications.")
                }
            }
        }

        override fun onCharacteristicChanged(gatt: BluetoothGatt?, characteristic: BluetoothGattCharacteristic?) {
            if (characteristic?.uuid == CHAR_TELEMETRY) {
                val rawCsv = characteristic.value?.toString(Charsets.UTF_8)
                Log.d(TAG, "Received BLE Telemetry: $rawCsv")
                _telemetryData.value = rawCsv
            }
        }
    }
}
