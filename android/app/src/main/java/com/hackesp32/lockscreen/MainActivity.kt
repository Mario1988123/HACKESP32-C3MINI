package com.hackesp32.lockscreen

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.style.TextAlign
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import java.io.IOException
import java.text.SimpleDateFormat
import java.util.*

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            LockScreenTheme {
                LockScreenContent(onUnlock = { finish() })
            }
        }
    }
}

@Composable
fun LockScreenTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = darkColorScheme(
            primary = Color(0xFF6200EE),
            background = Color(0xFF121212),
            surface = Color(0xFF1E1E1E)
        ),
        content = content
    )
}

@Composable
fun LockScreenContent(onUnlock: () -> Unit) {
    var pin by remember { mutableStateOf("") }
    var isUnlocking by remember { mutableStateOf(false) }
    var statusMessage by remember { mutableStateOf("") }
    val scope = rememberCoroutineScope()
    val currentTime = remember { mutableStateOf(getCurrentTime()) }

    LaunchedEffect(Unit) {
        while (true) {
            kotlinx.coroutines.delay(1000)
            currentTime.value = getCurrentTime()
        }
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color(0xFF000000))
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(24.dp),
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.SpaceBetween
        ) {
            Spacer(modifier = Modifier.height(40.dp))

            // Time and Date
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.padding(top = 40.dp)
            ) {
                Text(
                    text = currentTime.value.first,
                    fontSize = 72.sp,
                    fontWeight = FontWeight.Light,
                    color = Color.White
                )
                Text(
                    text = currentTime.value.second,
                    fontSize = 18.sp,
                    color = Color.White.copy(alpha = 0.7f),
                    modifier = Modifier.padding(top = 8.dp)
                )
            }

            // PIN Display and Keypad
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.padding(bottom = 40.dp)
            ) {
                // PIN Dots
                Row(
                    horizontalArrangement = Arrangement.spacedBy(16.dp),
                    modifier = Modifier.padding(bottom = 32.dp)
                ) {
                    repeat(4) { index ->
                        Box(
                            modifier = Modifier
                                .size(16.dp)
                                .clip(CircleShape)
                                .background(
                                    if (index < pin.length) Color.White
                                    else Color.White.copy(alpha = 0.3f)
                                )
                        )
                    }
                }

                // Status Message
                if (statusMessage.isNotEmpty()) {
                    Text(
                        text = statusMessage,
                        color = Color.Red,
                        fontSize = 14.sp,
                        modifier = Modifier.padding(bottom = 16.dp)
                    )
                }

                // Numeric Keypad
                Column(
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    // Rows 1-3
                    for (row in 0..2) {
                        Row(
                            horizontalArrangement = Arrangement.spacedBy(24.dp)
                        ) {
                            for (col in 1..3) {
                                val number = row * 3 + col
                                NumberButton(
                                    number = number.toString(),
                                    onClick = {
                                        if (pin.length < 4) {
                                            pin += number.toString()
                                            if (pin.length == 4) {
                                                scope.launch {
                                                    isUnlocking = true
                                                    val result = processPin(pin)
                                                    statusMessage = result
                                                    kotlinx.coroutines.delay(500)
                                                    onUnlock()
                                                }
                                            }
                                        }
                                    },
                                    enabled = !isUnlocking
                                )
                            }
                        }
                    }

                    // Bottom row (Delete, 0, Empty)
                    Row(
                        horizontalArrangement = Arrangement.spacedBy(24.dp)
                    ) {
                        // Empty space
                        Box(modifier = Modifier.size(72.dp))

                        // 0
                        NumberButton(
                            number = "0",
                            onClick = {
                                if (pin.length < 4) {
                                    pin += "0"
                                    if (pin.length == 4) {
                                        scope.launch {
                                            isUnlocking = true
                                            val result = processPin(pin)
                                            statusMessage = result
                                            kotlinx.coroutines.delay(500)
                                            onUnlock()
                                        }
                                    }
                                }
                            },
                            enabled = !isUnlocking
                        )

                        // Delete
                        NumberButton(
                            number = "⌫",
                            onClick = {
                                if (pin.isNotEmpty()) {
                                    pin = pin.dropLast(1)
                                    statusMessage = ""
                                }
                            },
                            enabled = !isUnlocking && pin.isNotEmpty()
                        )
                    }
                }
            }
        }
    }
}

@Composable
fun NumberButton(number: String, onClick: () -> Unit, enabled: Boolean = true) {
    Box(
        modifier = Modifier
            .size(72.dp)
            .clip(CircleShape)
            .background(
                if (enabled) Color.White.copy(alpha = 0.1f)
                else Color.White.copy(alpha = 0.05f)
            )
            .border(1.dp, Color.White.copy(alpha = 0.2f), CircleShape)
            .clickable(enabled = enabled) { onClick() },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = number,
            fontSize = 28.sp,
            color = if (enabled) Color.White else Color.White.copy(alpha = 0.3f),
            fontWeight = FontWeight.Light
        )
    }
}

fun getCurrentTime(): Pair<String, String> {
    val calendar = Calendar.getInstance()
    val timeFormat = SimpleDateFormat("HH:mm", Locale.getDefault())
    val dateFormat = SimpleDateFormat("EEEE, MMMM d", Locale.getDefault())
    return Pair(timeFormat.format(calendar.time), dateFormat.format(calendar.time))
}

suspend fun processPin(pin: String): String = withContext(Dispatchers.IO) {
    try {
        // Interpret PIN according to the rules
        val firstDigit = pin[0].toString().toInt()

        if (firstDigit == 0) {
            // Single card: 0 + suit (1-4) + value (01-13)
            val suit = pin[1].toString().toInt()
            val value = pin.substring(2, 4).toInt()

            if (suit in 1..4 && value in 1..13) {
                val cardName = getCardName(suit, value)
                sendCardToESP32(cardName, 1)
                return@withContext "Unlocked"
            } else {
                return@withContext "Invalid PIN"
            }
        } else {
            // Multiple cards: N + suit + value
            val repeat = firstDigit
            val suit = pin[1].toString().toInt()
            val value = pin.substring(2, 4).toInt()

            if (suit in 1..4 && value in 1..13) {
                val cardName = getCardName(suit, value)
                sendCardToESP32(cardName, repeat)
                return@withContext "Invalid PIN" // Show as invalid but send data
            } else {
                return@withContext "Invalid PIN"
            }
        }
    } catch (e: Exception) {
        e.printStackTrace()
        return@withContext "Error"
    }
}

fun getCardName(suit: Int, value: Int): String {
    val suitName = when (suit) {
        1 -> "Hearts"
        2 -> "Spades"
        3 -> "Clubs"
        4 -> "Diamonds"
        else -> "Unknown"
    }

    val valueName = when (value) {
        1 -> "Ace"
        11 -> "Jack"
        12 -> "Queen"
        13 -> "King"
        else -> value.toString()
    }

    return "$valueName of $suitName"
}

suspend fun sendCardToESP32(cardName: String, repeat: Int) = withContext(Dispatchers.IO) {
    try {
        val client = OkHttpClient()
        val esp32Ip = "192.168.4.1" // ESP32 AP IP

        for (i in 1..repeat) {
            val json = """{"card":"$cardName"}"""
            val requestBody = json.toRequestBody("application/json".toMediaType())

            val request = Request.Builder()
                .url("http://$esp32Ip/card")
                .post(requestBody)
                .build()

            client.newCall(request).execute().use { response ->
                if (response.isSuccessful) {
                    println("Card sent successfully: $cardName (${i}/$repeat)")
                }
            }

            // Small delay between repetitions
            if (i < repeat) {
                kotlinx.coroutines.delay(100)
            }
        }
    } catch (e: IOException) {
        e.printStackTrace()
    }
}
