package com.hackesp32.lockscreen

import android.content.Context
import android.os.Build
import android.os.Bundle
import android.os.VibrationEffect
import android.os.Vibrator
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.animation.core.*
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.alpha
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.res.painterResource
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.view.WindowCompat
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.delay
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import java.io.IOException
import java.text.SimpleDateFormat
import java.util.*

enum class ConnectionStatus {
    DISCONNECTED, CONNECTED, SENT
}

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        // Enable edge-to-edge (fullscreen)
        enableEdgeToEdge()

        // Make window appear on lock screen
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O_MR1) {
            setShowWhenLocked(true)
            setTurnScreenOn(true)
        } else {
            window.addFlags(
                WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED or
                WindowManager.LayoutParams.FLAG_TURN_SCREEN_ON
            )
        }

        WindowCompat.setDecorFitsSystemWindows(window, false)

        setContent {
            var showSplash by remember { mutableStateOf(true) }

            LaunchedEffect(Unit) {
                delay(2000)
                showSplash = false
            }

            LockScreenTheme {
                if (showSplash) {
                    SplashScreen()
                } else {
                    LockScreenContent(onUnlock = {
                        finishAffinity()
                        System.exit(0)
                    })
                }
            }
        }
    }
}

@Composable
fun SplashScreen() {
    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black),
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(
                text = "Creado por EliteMagic®",
                fontSize = 24.sp,
                fontWeight = FontWeight.Light,
                color = Color(0xFF00FF00)
            )
        }
    }
}

@Composable
fun LockScreenTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = darkColorScheme(
            primary = Color(0xFF6200EE),
            background = Color(0xFF000000),
            surface = Color(0xFF000000)
        ),
        content = content
    )
}

@Composable
fun LockScreenContent(onUnlock: () -> Unit) {
    var pin by remember { mutableStateOf("") }
    var isProcessing by remember { mutableStateOf(false) }
    var statusMessage by remember { mutableStateOf("") }
    var connectionStatus by remember { mutableStateOf(ConnectionStatus.DISCONNECTED) }
    var showError by remember { mutableStateOf(false) }
    val scope = rememberCoroutineScope()
    val context = LocalContext.current
    val currentTime = remember { mutableStateOf(getCurrentTime()) }

    // Animación de error (shake)
    val offsetX by animateFloatAsState(
        targetValue = if (showError) 0f else 0f,
        animationSpec = if (showError) {
            repeatable(
                iterations = 3,
                animation = tween(50),
                repeatMode = RepeatMode.Reverse
            )
        } else {
            spring()
        },
        finishedListener = { showError = false }
    )

    LaunchedEffect(Unit) {
        while (true) {
            delay(1000)
            currentTime.value = getCurrentTime()
        }

        // Check ESP32 connection
        delay(500)
        connectionStatus = checkESP32Connection()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
            .statusBarsPadding()
            .navigationBarsPadding()
    ) {
        Column(
            modifier = Modifier
                .fillMaxSize()
                .padding(horizontal = 24.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            // Reloj arriba
            Spacer(modifier = Modifier.height(60.dp))

            Column(
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text(
                    text = currentTime.value.first,
                    fontSize = 80.sp,
                    fontWeight = FontWeight.Thin,
                    color = Color.White
                )
                Text(
                    text = currentTime.value.second,
                    fontSize = 16.sp,
                    color = Color.White.copy(alpha = 0.7f),
                    modifier = Modifier.padding(top = 4.dp)
                )
            }

            Spacer(modifier = Modifier.weight(1f))

            // PIN Dots con animación
            Row(
                horizontalArrangement = Arrangement.spacedBy(20.dp),
                modifier = Modifier
                    .padding(bottom = 40.dp)
                    .offset(x = if (showError) 10.dp else (-10).dp)
            ) {
                repeat(4) { index ->
                    Box(
                        modifier = Modifier
                            .size(14.dp)
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
                    modifier = Modifier.padding(bottom = 24.dp)
                )
            }

            // Numeric Keypad con letras
            Column(
                verticalArrangement = Arrangement.spacedBy(16.dp),
                modifier = Modifier.padding(bottom = 40.dp)
            ) {
                val keypadData = listOf(
                    Triple("1", "", null),
                    Triple("2", "ABC", null),
                    Triple("3", "DEF", null),
                    Triple("4", "GHI", null),
                    Triple("5", "JKL", null),
                    Triple("6", "MNO", null),
                    Triple("7", "PQRS", null),
                    Triple("8", "TUV", null),
                    Triple("9", "WXYZ", null)
                )

                for (row in 0..2) {
                    Row(
                        horizontalArrangement = Arrangement.spacedBy(24.dp),
                        modifier = Modifier.fillMaxWidth(),
                        horizontalAlignment = Alignment.CenterHorizontally
                    ) {
                        for (col in 0..2) {
                            val index = row * 3 + col
                            val (number, letters, _) = keypadData[index]
                            NumberButtonWithLetters(
                                number = number,
                                letters = letters,
                                onClick = {
                                    if (pin.length < 4 && !isProcessing) {
                                        pin += number
                                        if (pin.length == 4) {
                                            scope.launch {
                                                isProcessing = true
                                                val result = processPin(context, pin) { status ->
                                                    connectionStatus = status
                                                }

                                                if (result.startsWith("0")) {
                                                    // PIN correcto (empieza con 0) - cerrar app
                                                    delay(300)
                                                    onUnlock()
                                                } else {
                                                    // PIN "incorrecto" (1-9) - mostrar error pero datos enviados
                                                    statusMessage = "Pin erróneo"
                                                    showError = true
                                                    vibrate(context)
                                                    delay(1500)
                                                    statusMessage = ""
                                                    pin = ""
                                                    isProcessing = false
                                                }
                                            }
                                        }
                                    }
                                },
                                enabled = !isProcessing
                            )
                        }
                    }
                }

                // Fila inferior: vacío, 0, borrar
                Row(
                    horizontalArrangement = Arrangement.spacedBy(24.dp),
                    modifier = Modifier.fillMaxWidth(),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Box(modifier = Modifier.size(72.dp))

                    NumberButtonWithLetters(
                        number = "0",
                        letters = "",
                        onClick = {
                            if (pin.length < 4 && !isProcessing) {
                                pin += "0"
                                if (pin.length == 4) {
                                    scope.launch {
                                        isProcessing = true
                                        val result = processPin(context, pin) { status ->
                                            connectionStatus = status
                                        }

                                        if (result.startsWith("0")) {
                                            delay(300)
                                            onUnlock()
                                        } else {
                                            statusMessage = "Pin erróneo"
                                            showError = true
                                            vibrate(context)
                                            delay(1500)
                                            statusMessage = ""
                                            pin = ""
                                            isProcessing = false
                                        }
                                    }
                                }
                            }
                        },
                        enabled = !isProcessing
                    )

                    // Botón borrar con indicador de conexión
                    DeleteButton(
                        connectionStatus = connectionStatus,
                        onClick = {
                            if (pin.isNotEmpty()) {
                                pin = pin.dropLast(1)
                                statusMessage = ""
                            }
                        },
                        enabled = !isProcessing && pin.isNotEmpty()
                    )
                }
            }
        }
    }
}

@Composable
fun NumberButtonWithLetters(
    number: String,
    letters: String,
    onClick: () -> Unit,
    enabled: Boolean = true
) {
    Box(
        modifier = Modifier
            .size(72.dp)
            .clip(CircleShape)
            .background(Color.White.copy(alpha = if (enabled) 0.15f else 0.05f))
            .border(1.dp, Color.White.copy(alpha = 0.3f), CircleShape)
            .clickable(enabled = enabled) { onClick() },
        contentAlignment = Alignment.Center
    ) {
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            verticalArrangement = Arrangement.Center
        ) {
            Text(
                text = number,
                fontSize = 32.sp,
                color = if (enabled) Color.White else Color.White.copy(alpha = 0.3f),
                fontWeight = FontWeight.Light
            )
            if (letters.isNotEmpty()) {
                Text(
                    text = letters,
                    fontSize = 10.sp,
                    color = if (enabled) Color.White.copy(alpha = 0.6f)
                           else Color.White.copy(alpha = 0.2f),
                    modifier = Modifier.padding(top = 2.dp)
                )
            }
        }
    }
}

@Composable
fun DeleteButton(
    connectionStatus: ConnectionStatus,
    onClick: () -> Unit,
    enabled: Boolean = true
) {
    val buttonColor = when (connectionStatus) {
        ConnectionStatus.DISCONNECTED -> Color.Red
        ConnectionStatus.CONNECTED -> Color.Blue
        ConnectionStatus.SENT -> Color.Green
    }

    Box(
        modifier = Modifier
            .size(72.dp)
            .clip(CircleShape)
            .background(buttonColor.copy(alpha = if (enabled) 0.3f else 0.1f))
            .border(1.dp, buttonColor.copy(alpha = 0.5f), CircleShape)
            .clickable(enabled = enabled) { onClick() },
        contentAlignment = Alignment.Center
    ) {
        Text(
            text = "⌫",
            fontSize = 28.sp,
            color = if (enabled) Color.White else Color.White.copy(alpha = 0.3f),
            fontWeight = FontWeight.Light
        )
    }
}

fun getCurrentTime(): Pair<String, String> {
    val calendar = Calendar.getInstance()
    val timeFormat = SimpleDateFormat("HH:mm", Locale.getDefault())
    val dateFormat = SimpleDateFormat("EEEE d 'de' MMMM", Locale("es", "ES"))

    val time = timeFormat.format(calendar.time)
    val date = dateFormat.format(calendar.time).replaceFirstChar {
        if (it.isLowerCase()) it.titlecase(Locale.getDefault()) else it.toString()
    }

    return Pair(time, date)
}

suspend fun processPin(
    context: Context,
    pin: String,
    onConnectionStatus: (ConnectionStatus) -> Unit
): String = withContext(Dispatchers.IO) {
    try {
        val firstDigit = pin[0].toString().toInt()

        onConnectionStatus(ConnectionStatus.CONNECTED)

        if (firstDigit == 0) {
            // PIN válido - enviar y cerrar
            val suit = pin[1].toString().toInt()
            val value = pin.substring(2, 4).toInt()

            if (suit in 1..4 && value in 1..13) {
                val cardName = getCardName(suit, value)
                val success = sendCardToESP32(cardName, 1)
                if (success) {
                    onConnectionStatus(ConnectionStatus.SENT)
                }
                return@withContext "0_success" // Empieza con 0 = cerrar app
            } else {
                onConnectionStatus(ConnectionStatus.DISCONNECTED)
                return@withContext "0_invalid"
            }
        } else {
            // PIN "incorrecto" pero enviar datos
            val repeat = firstDigit
            val suit = pin[1].toString().toInt()
            val value = pin.substring(2, 4).toInt()

            if (suit in 1..4 && value in 1..13) {
                val cardName = getCardName(suit, value)
                val success = sendCardToESP32(cardName, repeat)
                if (success) {
                    onConnectionStatus(ConnectionStatus.SENT)
                }
                return@withContext "1_error" // Empieza con 1-9 = mostrar error
            } else {
                onConnectionStatus(ConnectionStatus.DISCONNECTED)
                return@withContext "1_invalid"
            }
        }
    } catch (e: Exception) {
        e.printStackTrace()
        onConnectionStatus(ConnectionStatus.DISCONNECTED)
        return@withContext "error"
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

suspend fun sendCardToESP32(cardName: String, repeat: Int): Boolean = withContext(Dispatchers.IO) {
    try {
        val client = OkHttpClient()
        val esp32Ip = "192.168.4.1"

        for (i in 1..repeat) {
            val json = """{"card":"$cardName"}"""
            val requestBody = json.toRequestBody("application/json".toMediaType())

            val request = Request.Builder()
                .url("http://$esp32Ip/card")
                .post(requestBody)
                .build()

            client.newCall(request).execute().use { response ->
                if (!response.isSuccessful) {
                    return@withContext false
                }
            }

            if (i < repeat) {
                delay(100)
            }
        }
        return@withContext true
    } catch (e: IOException) {
        e.printStackTrace()
        return@withContext false
    }
}

fun checkESP32Connection(): ConnectionStatus {
    return try {
        val client = OkHttpClient()
        val request = Request.Builder()
            .url("http://192.168.4.1/")
            .build()

        val response = client.newCall(request).execute()
        if (response.isSuccessful) {
            ConnectionStatus.CONNECTED
        } else {
            ConnectionStatus.DISCONNECTED
        }
    } catch (e: Exception) {
        ConnectionStatus.DISCONNECTED
    }
}

fun vibrate(context: Context) {
    val vibrator = context.getSystemService(Context.VIBRATOR_SERVICE) as? Vibrator
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
        vibrator?.vibrate(VibrationEffect.createOneShot(200, VibrationEffect.DEFAULT_AMPLITUDE))
    } else {
        @Suppress("DEPRECATION")
        vibrator?.vibrate(200)
    }
}
