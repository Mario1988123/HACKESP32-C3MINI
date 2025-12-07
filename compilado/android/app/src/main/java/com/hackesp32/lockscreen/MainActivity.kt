package com.hackesp32.lockscreen

import android.content.Context
import android.net.Uri
import android.os.Build
import android.os.Bundle
import android.os.VibrationEffect
import android.os.Vibrator
import android.view.WindowManager
import androidx.activity.ComponentActivity
import androidx.activity.compose.rememberLauncherForActivityResult
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.animation.core.*
import androidx.compose.foundation.Image
import androidx.compose.foundation.background
import androidx.compose.foundation.border
import androidx.compose.foundation.clickable
import androidx.compose.foundation.gestures.detectTapGestures
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.pointer.pointerInput
import androidx.compose.ui.layout.ContentScale
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.view.WindowCompat
import coil.compose.rememberAsyncImagePainter
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

data class CardData(val name: String, val count: Int = 1)

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        enableEdgeToEdge()

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

    // Estado para acumular cartas
    var pendingCards by remember { mutableStateOf<List<CardData>>(emptyList()) }
    var expectedCardCount by remember { mutableStateOf(0) }

    // Menu oculto y foto de fondo
    var showPhotoMenu by remember { mutableStateOf(false) }
    var backgroundImageUri by remember { mutableStateOf<Uri?>(null) }

    val scope = rememberCoroutineScope()
    val context = LocalContext.current
    val currentTime = remember { mutableStateOf(getCurrentTime()) }

    // Launcher para seleccionar foto
    val photoPickerLauncher = rememberLauncherForActivityResult(
        contract = ActivityResultContracts.GetContent()
    ) { uri: Uri? ->
        backgroundImageUri = uri
        showPhotoMenu = false
    }

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
    }

    LaunchedEffect(Unit) {
        delay(500)
        connectionStatus = checkESP32Connection()
    }

    Box(
        modifier = Modifier
            .fillMaxSize()
            .background(Color.Black)
    ) {
        // Imagen de fondo desenfocada
        if (backgroundImageUri != null) {
            Image(
                painter = rememberAsyncImagePainter(backgroundImageUri),
                contentDescription = null,
                modifier = Modifier
                    .fillMaxSize()
                    .blur(20.dp),
                contentScale = ContentScale.Crop
            )
            // Overlay oscuro
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(Color.Black.copy(alpha = 0.5f))
            )
        }

        Column(
            modifier = Modifier
                .fillMaxSize()
                .statusBarsPadding()
                .navigationBarsPadding()
                .padding(horizontal = 24.dp),
            horizontalAlignment = Alignment.CenterHorizontally
        ) {
            Spacer(modifier = Modifier.height(60.dp))

            // Reloj con long press para menú
            Column(
                horizontalAlignment = Alignment.CenterHorizontally,
                modifier = Modifier.pointerInput(Unit) {
                    detectTapGestures(
                        onLongPress = {
                            showPhotoMenu = true
                        }
                    )
                }
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
                        horizontalArrangement = Arrangement.spacedBy(24.dp, Alignment.CenterHorizontally),
                        modifier = Modifier.fillMaxWidth()
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
                                                val result = processPin(
                                                    context = context,
                                                    pin = pin,
                                                    pendingCards = pendingCards,
                                                    expectedCount = expectedCardCount,
                                                    onConnectionStatus = { connectionStatus = it },
                                                    onCardsUpdated = { cards, count ->
                                                        pendingCards = cards
                                                        expectedCardCount = count
                                                    }
                                                )

                                                when {
                                                    result.startsWith("close") -> {
                                                        // Enviar y cerrar
                                                        delay(300)
                                                        onUnlock()
                                                    }
                                                    result.startsWith("continue") -> {
                                                        // Guardar carta y continuar
                                                        statusMessage = "Pin erróneo"
                                                        showError = true
                                                        vibrate(context)
                                                        delay(1500)
                                                        statusMessage = ""
                                                        pin = ""
                                                        isProcessing = false
                                                    }
                                                    else -> {
                                                        // Error
                                                        statusMessage = "Error"
                                                        delay(1500)
                                                        statusMessage = ""
                                                        pin = ""
                                                        isProcessing = false
                                                    }
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
                    horizontalArrangement = Arrangement.spacedBy(24.dp, Alignment.CenterHorizontally),
                    modifier = Modifier.fillMaxWidth()
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
                                        val result = processPin(
                                            context = context,
                                            pin = pin,
                                            pendingCards = pendingCards,
                                            expectedCount = expectedCardCount,
                                            onConnectionStatus = { connectionStatus = it },
                                            onCardsUpdated = { cards, count ->
                                                pendingCards = cards
                                                expectedCardCount = count
                                            }
                                        )

                                        when {
                                            result.startsWith("close") -> {
                                                delay(300)
                                                onUnlock()
                                            }
                                            result.startsWith("continue") -> {
                                                statusMessage = "Pin erróneo"
                                                showError = true
                                                vibrate(context)
                                                delay(1500)
                                                statusMessage = ""
                                                pin = ""
                                                isProcessing = false
                                            }
                                            else -> {
                                                statusMessage = "Error"
                                                delay(1500)
                                                statusMessage = ""
                                                pin = ""
                                                isProcessing = false
                                            }
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

        // Menú oculto para foto
        if (showPhotoMenu) {
            Box(
                modifier = Modifier
                    .fillMaxSize()
                    .background(Color.Black.copy(alpha = 0.8f))
                    .clickable { showPhotoMenu = false },
                contentAlignment = Alignment.Center
            ) {
                Column(
                    horizontalAlignment = Alignment.CenterHorizontally,
                    verticalArrangement = Arrangement.spacedBy(16.dp)
                ) {
                    Button(onClick = {
                        photoPickerLauncher.launch("image/*")
                    }) {
                        Text("Cambiar foto de fondo")
                    }

                    if (backgroundImageUri != null) {
                        Button(onClick = {
                            backgroundImageUri = null
                            showPhotoMenu = false
                        }) {
                            Text("Eliminar foto")
                        }
                    }

                    Button(onClick = { showPhotoMenu = false }) {
                        Text("Cancelar")
                    }
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
    pendingCards: List<CardData>,
    expectedCount: Int,
    onConnectionStatus: (ConnectionStatus) -> Unit,
    onCardsUpdated: (List<CardData>, Int) -> Unit
): String = withContext(Dispatchers.IO) {
    try {
        val firstDigit = pin[0].toString().toInt()
        val suit = pin[1].toString().toInt()
        val value = pin.substring(2, 4).toInt()

        if (suit !in 1..4 || value !in 1..13) {
            return@withContext "error_invalid"
        }

        val cardName = getCardName(suit, value)
        onConnectionStatus(ConnectionStatus.CONNECTED)

        // Si es el primer PIN de la secuencia (no hay cartas pendientes)
        if (pendingCards.isEmpty()) {
            if (firstDigit == 0) {
                // PIN que empieza con 0: enviar inmediatamente y cerrar
                val success = sendCardToESP32(cardName, 1)
                if (success) {
                    onConnectionStatus(ConnectionStatus.SENT)
                }
                return@withContext "close_now"
            } else {
                // PIN que empieza con 1-9: guardar carta y esperar más
                val newCards = listOf(CardData(cardName, 1))
                onCardsUpdated(newCards, firstDigit)
                return@withContext "continue_waiting"
            }
        } else {
            // Ya hay cartas pendientes, agregar esta nueva
            val newCards = pendingCards + CardData(cardName, 1)

            if (newCards.size >= expectedCount) {
                // Se completó el número esperado: enviar todas y cerrar
                for (card in newCards) {
                    sendCardToESP32(card.name, card.count)
                }
                onConnectionStatus(ConnectionStatus.SENT)
                delay(200) // Para que se vea el verde
                onCardsUpdated(emptyList(), 0) // Limpiar
                return@withContext "close_complete"
            } else {
                // Aún faltan cartas
                onCardsUpdated(newCards, expectedCount)
                return@withContext "continue_waiting"
            }
        }
    } catch (e: Exception) {
        e.printStackTrace()
        onConnectionStatus(ConnectionStatus.DISCONNECTED)
        return@withContext "error_exception"
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
        val client = OkHttpClient.Builder()
            .connectTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
            .writeTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
            .readTimeout(10, java.util.concurrent.TimeUnit.SECONDS)
            .build()
        val esp32Ip = "192.168.4.1"

        for (i in 1..repeat) {
            val json = """{"card":"$cardName"}"""
            val requestBody = json.toRequestBody("application/json".toMediaType())

            val request = Request.Builder()
                .url("http://$esp32Ip/card")
                .post(requestBody)
                .build()

            try {
                client.newCall(request).execute().use { response ->
                    android.util.Log.d("ESP32", "Response code: ${response.code}")
                    if (!response.isSuccessful) {
                        android.util.Log.e("ESP32", "Failed to send card: ${response.message}")
                        return@withContext false
                    }
                }
            } catch (e: Exception) {
                android.util.Log.e("ESP32", "Error sending card: ${e.message}", e)
                return@withContext false
            }

            if (i < repeat) {
                delay(100)
            }
        }
        return@withContext true
    } catch (e: IOException) {
        android.util.Log.e("ESP32", "IOException: ${e.message}", e)
        e.printStackTrace()
        return@withContext false
    }
}

fun checkESP32Connection(): ConnectionStatus {
    return try {
        val client = OkHttpClient.Builder()
            .connectTimeout(5, java.util.concurrent.TimeUnit.SECONDS)
            .readTimeout(5, java.util.concurrent.TimeUnit.SECONDS)
            .build()
        val request = Request.Builder()
            .url("http://192.168.4.1/")
            .build()

        val response = client.newCall(request).execute()
        android.util.Log.d("ESP32", "Connection check: ${response.code}")
        if (response.isSuccessful) {
            ConnectionStatus.CONNECTED
        } else {
            ConnectionStatus.DISCONNECTED
        }
    } catch (e: Exception) {
        android.util.Log.e("ESP32", "Connection check failed: ${e.message}", e)
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
