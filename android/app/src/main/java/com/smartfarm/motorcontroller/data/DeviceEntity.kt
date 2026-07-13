package com.smartfarm.motorcontroller.data

import androidx.room.Entity
import androidx.room.PrimaryKey

@Entity(tableName = "devices")
data class DeviceEntity(
    @PrimaryKey val id: String,
    val name: String,
    val simNumber: String,
    val ownerId: String,
    val lastState: String,
    val isOnline: Boolean,
    val activeLanguage: Int, // 1 = English, 2 = Telugu, 3 = Hindi
    val pumpType: Int,       // 1 = Single Phase, 2 = Three Phase, 3 = Solar VFD
    val syncStatus: Int      // 0 = SYNCED, 1 = PENDING_OFFLINE_CHANGE
)
