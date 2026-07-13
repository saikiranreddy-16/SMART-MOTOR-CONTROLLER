package com.smartfarm.motorcontroller.data

import androidx.room.Entity
import androidx.room.ForeignKey
import androidx.room.PrimaryKey

@Entity(
    tableName = "telemetry_logs",
    foreignKeys = [
        ForeignKey(
            entity = DeviceEntity::class,
            parentColumns = ["id"],
            childColumns = ["deviceId"],
            onDelete = ForeignKey.CASCADE
        )
    ]
)
data class TelemetryLogEntity(
    @PrimaryKey(autoGenerate = true) val id: Long = 0,
    val deviceId: String,
    val timestamp: Long,
    val avgVoltage: Float,
    val avgCurrent: Float,
    val waterLevel: Float,
    val soilMoisture: Float
)
