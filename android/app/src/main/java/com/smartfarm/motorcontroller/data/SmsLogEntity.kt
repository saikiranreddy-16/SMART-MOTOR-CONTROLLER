package com.smartfarm.motorcontroller.data

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity(tableName = "sms_audit_logs")
data class SmsLogEntity(
    @PrimaryKey(autoGenerate = true) val id: Long = 0,
    val deviceId: String,
    val timestamp: Long,
    val direction: String, // "SENT" or "RECEIVED"
    val rawMessage: String,
    val status: String     // "PENDING", "DELIVERED", "PARSED_OK", "PARSE_ERROR"
)
