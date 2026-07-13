package com.smartfarm.motorcontroller.sms

import android.telephony.SmsManager
import android.util.Log

class SmsSender {
    fun sendCommand(phoneNumber: String, command: String, onSent: (Boolean) -> Unit) {
        try {
            val smsManager = SmsManager.getDefault()
            smsManager.sendTextMessage(phoneNumber, null, command, null, null)
            Log.d("SmsSender", "SMS Command '$command' sent to $phoneNumber")
            onSent(true)
        } catch (e: Exception) {
            Log.e("SmsSender", "Failed to send SMS command", e)
            onSent(false)
        }
    }
}
