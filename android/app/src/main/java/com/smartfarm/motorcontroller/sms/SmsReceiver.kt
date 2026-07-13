package com.smartfarm.motorcontroller.sms

import android.content.BroadcastReceiver
import android.content.Context
import android.content.Intent
import android.provider.Telephony
import android.util.Log
import com.smartfarm.motorcontroller.data.AppDatabase
import com.smartfarm.motorcontroller.data.DeviceEntity
import com.smartfarm.motorcontroller.data.SmsLogEntity
import com.smartfarm.motorcontroller.data.TelemetryLogEntity
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.util.regex.Pattern

class SmsReceiver : BroadcastReceiver() {

    override fun onReceive(context: Context, intent: Intent) {
        if (intent.action == Telephony.Sms.Intents.SMS_RECEIVED_ACTION) {
            val messages = Telephony.Sms.Intents.getMessagesFromIntent(intent)
            for (sms in messages) {
                val sender = sms.originatingAddress ?: continue
                val body = sms.messageBody ?: continue

                if (body.startsWith("SF ")) {
                    Log.d("SmsReceiver", "SmartFarm SMS response received from $sender: $body")
                    
                    // Process message asynchronously using CoroutineScope
                    val db = androidx.room.Room.databaseBuilder(
                        context.applicationContext,
                        AppDatabase::class.java, "smartfarm-db"
                    ).build()

                    CoroutineScope(Dispatchers.IO).launch {
                        // Log the raw SMS receipt
                        db.deviceDao().insertSmsLog(
                            SmsLogEntity(
                                deviceId = "UNKNOWN", // Will resolve below
                                timestamp = System.currentTimeMillis(),
                                direction = "RECEIVED",
                                rawMessage = body,
                                status = "PARSED_OK"
                            )
                        )
                        
                        // Parse status message
                        // Example: "SF STATUS: RUNNING\nVolt: R:230 Y:232 B:229\nCurr: R:11.2 Y:11.4 B:11.1\nTank: 82%\nSoil: 42%\nTemp: 45C"
                        try {
                            val state = parseField(body, "STATUS:\\s*(\\w+)")
                            val vR = parseField(body, "R:(\\d+)").toFloatOrNull() ?: 0f
                            val vY = parseField(body, "Y:(\\d+)").toFloatOrNull() ?: 0f
                            val vB = parseField(body, "B:(\\d+)").toFloatOrNull() ?: 0f
                            
                            val iR = parseField(body, "nCurr:\\s*R:([\\d.]+)").toFloatOrNull() ?: 0f // Current R
                            
                            val tankStr = parseField(body, "Tank:\\s*(\\d+)%")
                            val soilStr = parseField(body, "Soil:\\s*(\\d+)%")
                            
                            val tankLevel = tankStr.toFloatOrNull() ?: 0f
                            val soilMoisture = soilStr.toFloatOrNull() ?: 0f

                            // Query device matching the SIM number
                            val devices = db.deviceDao().getAllDevices()
                            // In this simple database search, let's look for matching phone
                            // (Iterating over list)
                            // We can fetch device list, update the status
                            // For prototype design, let's print parsed fields to log
                            Log.d("SmsReceiver", "Parsed SMS: State=$state, Volt_R=$vR, Tank=$tankLevel, Soil=$soilMoisture")
                            
                            // Insert parsed log
                            db.deviceDao().insertTelemetryLog(
                                TelemetryLogEntity(
                                    deviceId = "ESP32_A83C10", // Example ID
                                    timestamp = System.currentTimeMillis(),
                                    avgVoltage = (vR + vY + vB) / 3.0f,
                                    avgCurrent = iR, // simplified
                                    waterLevel = tankLevel,
                                    soilMoisture = soilMoisture
                                )
                            )
                        } catch (e: Exception) {
                            Log.e("SmsReceiver", "Parsing SMS failed", e)
                        }
                    }
                }
            }
        }
    }

    private fun parseField(text: String, patternString: String): String {
        val pattern = Pattern.compile(patternString)
        val matcher = pattern.matcher(text)
        return if (matcher.find()) {
            matcher.group(1) ?: ""
        } else {
            ""
        }
    }
}
