package com.smartfarm.motorcontroller.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp

@OptIn(ExperimentalMaterial3Api::class)
@Composable
fun DashboardScreen(
    motorState: String,           // "OFF", "RUNNING", "FAULT"
    powerAvailable: Boolean,       // true = AC Power OK, false = No Power
    waterLevel: Float,             // 0 - 100%
    weatherForecast: String,      // "Rain Expected", "Sunny", "Cloudy"
    aiRecommendation: String,     // "Irrigation can be postponed"
    onStartClick: () -> Unit,
    onStopClick: () -> Unit,
    onSettingsClick: () -> Unit
) {
    // Theme colors
    val themeGreen = Color(0xFF2E7D32)
    val themeRed = Color(0xFFC62828)
    val themeOrange = Color(0xFFE65100)

    val stateColor = when (motorState) {
        "RUNNING" -> themeGreen
        "FAULT" -> themeOrange
        else -> themeRed
    }

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Smart Motor Controller", fontWeight = FontWeight.Bold) },
                actions = {
                    IconButton(onClick = onSettingsClick) {
                        Icon(Icons.Default.Settings, contentDescription = "Settings")
                    }
                }
            )
        }
    ) { paddingValues ->
        Column(
            modifier = Modifier
                .fillMaxSize()
                .background(MaterialTheme.colorScheme.background)
                .padding(paddingValues)
                .padding(20.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            
            // 1. Motor Status Panel (Large, Clear)
            Card(
                modifier = Modifier.fillMaxWidth(),
                shape = RoundedCornerShape(24.dp),
                colors = CardDefaults.cardColors(containerColor = stateColor.copy(alpha = 0.08f))
            ) {
                Column(
                    modifier = Modifier.padding(24.dp),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Text(
                        text = "MOTOR STATUS",
                        fontSize = 12.sp,
                        fontWeight = FontWeight.Bold,
                        color = Color.Gray
                    )
                    Spacer(modifier = Modifier.height(8.dp))
                    Text(
                        text = motorState,
                        fontSize = 32.sp,
                        fontWeight = FontWeight.Black,
                        color = stateColor
                    )
                }
            }

            // 2. Metrics (Water Level & Weather)
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.spacedBy(12.dp)
            ) {
                // Water Level Card
                Card(
                    modifier = Modifier.weight(1f),
                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant)
                ) {
                    Column(
                        modifier = Modifier.padding(16.dp),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Icon(Icons.Default.Info, contentDescription = "Water", tint = Color(0xFF0288D1), modifier = Modifier.size(28.dp))
                        Spacer(modifier = Modifier.height(4.dp))
                        Text("WATER LEVEL", fontSize = 10.sp, fontWeight = FontWeight.Bold, color = Color.Gray)
                        Text("${waterLevel.toInt()}%", fontSize = 20.sp, fontWeight = FontWeight.Bold, color = Color(0xFF0288D1))
                    }
                }

                // Weather Card
                Card(
                    modifier = Modifier.weight(1f),
                    colors = CardDefaults.cardColors(containerColor = MaterialTheme.colorScheme.surfaceVariant)
                ) {
                    Column(
                        modifier = Modifier.padding(16.dp),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        Icon(Icons.Default.Star, contentDescription = "Weather", tint = Color(0xFFF57C00), modifier = Modifier.size(28.dp))
                        Spacer(modifier = Modifier.height(4.dp))
                        Text("WEATHER", fontSize = 10.sp, fontWeight = FontWeight.Bold, color = Color.Gray)
                        Text(weatherForecast, fontSize = 16.sp, fontWeight = FontWeight.Bold)
                    }
                }
            }

            // 3. AI Advice Notification Bubble
            Card(
                modifier = Modifier.fillMaxWidth(),
                colors = CardDefaults.cardColors(containerColor = Color(0xFFE8F5E9)),
                shape = RoundedCornerShape(16.dp)
            ) {
                Row(
                    modifier = Modifier.padding(16.dp),
                    verticalAlignment = Alignment.CenterVertically
                ) {
                    Icon(Icons.Default.Check, contentDescription = "AI Advice", tint = themeGreen, modifier = Modifier.size(32.dp))
                    Spacer(modifier = Modifier.width(12.dp))
                    Column {
                        Text("AI RECOMMENDATION", fontSize = 10.sp, fontWeight = FontWeight.Bold, color = themeGreen)
                        Text(aiRecommendation, fontSize = 13.sp, color = Color.Black)
                    }
                }
            }

            Spacer(modifier = Modifier.weight(1f))

            // 4. Large Green START Button
            Button(
                onClick = onStartClick,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(72.dp),
                colors = ButtonDefaults.buttonColors(containerColor = themeGreen),
                shape = RoundedCornerShape(16.dp)
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(Icons.Default.PlayArrow, contentDescription = "Start", tint = Color.White, modifier = Modifier.size(32.dp))
                    Spacer(modifier = Modifier.width(12.dp))
                    Text("START MOTOR", fontSize = 20.sp, fontWeight = FontWeight.Bold, color = Color.White)
                }
            }

            // 5. Large Red STOP Button
            Button(
                onClick = onStopClick,
                modifier = Modifier
                    .fillMaxWidth()
                    .height(72.dp),
                colors = ButtonDefaults.buttonColors(containerColor = themeRed),
                shape = RoundedCornerShape(16.dp)
            ) {
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Icon(Icons.Default.Close, contentDescription = "Stop", tint = Color.White, modifier = Modifier.size(32.dp))
                    Spacer(modifier = Modifier.width(12.dp))
                    Text("STOP MOTOR", fontSize = 20.sp, fontWeight = FontWeight.Bold, color = Color.White)
                }
            }
        }
    }
}
