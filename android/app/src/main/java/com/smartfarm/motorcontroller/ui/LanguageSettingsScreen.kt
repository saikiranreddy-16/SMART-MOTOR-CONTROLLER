package com.smartfarm.motorcontroller.ui

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.ArrowBack
import androidx.compose.material.icons.filled.Check
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
fun LanguageSettingsScreen(
    currentLanguageCode: Int, // 1 = ENG, 2 = TEL, 3 = HIN
    onLanguageSelected: (Int) -> Unit,
    onBackClick: () -> Unit
) {
    var selectedLang by remember { mutableStateOf(currentLanguageCode) }
    val themeGreen = Color(0xFF2E7D32)

    Scaffold(
        topBar = {
            TopAppBar(
                title = { Text("Choose Language / भाषा चुनें", fontWeight = FontWeight.Bold) },
                navigationIcon = {
                    IconButton(onClick = onBackClick) {
                        Icon(Icons.Default.ArrowBack, contentDescription = "Back")
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
            verticalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            Text(
                text = "Select your preferred language for App and Phone Calls:",
                fontSize = 16.sp,
                fontWeight = FontWeight.Medium,
                color = MaterialTheme.colorScheme.onBackground
            )

            // Language Cards
            LanguageItemCard(
                languageName = "English",
                nativeLabel = "English",
                isSelected = selectedLang == 1,
                onClick = { selectedLang = 1 }
            )

            LanguageItemCard(
                languageName = "Telugu",
                nativeLabel = "తెలుగు",
                isSelected = selectedLang == 2,
                onClick = { selectedLang = 2 }
            )

            LanguageItemCard(
                languageName = "Hindi",
                nativeLabel = "हिन्दी",
                isSelected = selectedLang == 3,
                onClick = { selectedLang = 3 }
            )

            Spacer(modifier = Modifier.weight(1f))

            // Save Button
            Button(
                onClick = { onLanguageSelected(selectedLang) },
                modifier = Modifier
                    .fillMaxWidth()
                    .height(56.dp),
                colors = ButtonDefaults.buttonColors(containerColor = themeGreen),
                shape = RoundedCornerShape(12.dp)
            ) {
                Text(
                    text = "SAVE LANGUAGE / सुरक्षित करें",
                    fontSize = 16.sp,
                    fontWeight = FontWeight.Bold,
                    color = Color.White
                )
            }
        }
    }
}

@Composable
fun LanguageItemCard(
    languageName: String,
    nativeLabel: String,
    isSelected: Boolean,
    onClick: () -> Unit
) {
    val borderColor = if (isSelected) Color(0xFF2E7D32) else Color.LightGray
    val backColor = if (isSelected) Color(0xFFE8F5E9) else MaterialTheme.colorScheme.surface

    Card(
        modifier = Modifier
            .fillMaxWidth()
            .clickable(onClick = onClick),
        shape = RoundedCornerShape(12.dp),
        colors = CardDefaults.cardColors(containerColor = backColor),
        border = CardStroke(2.dp, borderColor)
    ) {
        Row(
            modifier = Modifier
                .padding(20.dp)
                .fillMaxWidth(),
            horizontalArrangement = Arrangement.SpaceBetween,
            verticalAlignment = Alignment.CenterVertically
        ) {
            Column {
                Text(
                    text = nativeLabel,
                    fontSize = 20.sp,
                    fontWeight = FontWeight.Bold,
                    color = if (isSelected) Color(0xFF2E7D32) else MaterialTheme.colorScheme.onSurface
                )
                Text(
                    text = languageName,
                    fontSize = 12.sp,
                    color = Color.Gray
                )
            }
            if (isSelected) {
                Icon(
                    imageVector = Icons.Default.Check,
                    contentDescription = "Selected",
                    tint = Color(0xFF2E7D32),
                    modifier = Modifier.size(28.dp)
                )
            }
        }
    }
}
