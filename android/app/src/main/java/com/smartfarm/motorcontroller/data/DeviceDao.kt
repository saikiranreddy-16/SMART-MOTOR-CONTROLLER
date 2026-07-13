package com.smartfarm.motorcontroller.data

import androidx.room.*
import kotlinx.coroutines.flow.Flow

@Dao
interface DeviceDao {

    @Query("SELECT * FROM devices")
    fun getAllDevices(): Flow<List<DeviceEntity>>

    @Query("SELECT * FROM devices WHERE id = :deviceId")
    suspend fun getDeviceById(deviceId: String): DeviceEntity?

    @Insert(onConflict = OnConflictStrategy.REPLACE)
    suspend fun insertDevice(device: DeviceEntity)

    @Update
    suspend fun updateDevice(device: DeviceEntity)

    @Delete
    suspend fun deleteDevice(device: DeviceEntity)

    // Telemetry Logs queries
    @Query("SELECT * FROM telemetry_logs WHERE deviceId = :deviceId ORDER BY timestamp DESC LIMIT 100")
    fun getRecentTelemetry(deviceId: String): Flow<List<TelemetryLogEntity>>

    @Insert
    suspend fun insertTelemetryLog(log: TelemetryLogEntity)

    // SMS Logs queries
    @Query("SELECT * FROM sms_audit_logs ORDER BY timestamp DESC")
    fun getAllSmsLogs(): Flow<List<SmsLogEntity>>

    @Insert
    suspend fun insertSmsLog(log: SmsLogEntity)
}
