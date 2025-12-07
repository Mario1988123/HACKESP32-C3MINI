# 🔒 Lock Screen Card - ESP32 Oculto con Pantalla de Bloqueo

Sistema de comunicación encubierta que utiliza una aplicación Android de pantalla de bloqueo para enviar cartas de póker al ESP32-C3 MINI vía WiFi.

## 📋 Descripción

Este proyecto simula una pantalla de bloqueo normal de Android, pero en realidad envía información de cartas de póker al ESP32 mediante códigos PIN. El sistema está diseñado para parecer completamente legítimo mientras transmite datos de manera discreta.

## 🎯 Funcionamiento

### Sistema de Codificación de PIN

El PIN de 4 dígitos se interpreta de la siguiente manera:

#### **Formato: XPVV**

- **X (primer dígito)**: Cantidad de repeticiones
  - `0` = Enviar una sola carta
  - `1-9` = Enviar la carta N veces

- **P (segundo dígito)**: Palo de la carta
  - `1` = ♥ Corazones (Hearts)
  - `2` = ♠ Picas (Spades)
  - `3` = ♣ Tréboles (Clubs)
  - `4` = ♦ Diamantes (Diamonds)

- **VV (dígitos 3 y 4)**: Valor de la carta
  - `01` = As (Ace)
  - `02-10` = Cartas numéricas
  - `11` = J (Jack)
  - `12` = Q (Queen)
  - `13` = K (King)

### Ejemplos de Uso

| PIN  | Interpretación | Resultado |
|------|----------------|-----------|
| `0101` | 0 repeticiones + ♥ + As | Envía **1x As de Corazones** y desbloquea |
| `0213` | 0 repeticiones + ♠ + K | Envía **1x Rey de Picas** y desbloquea |
| `0312` | 0 repeticiones + ♣ + Q | Envía **1x Reina de Tréboles** y desbloquea |
| `0407` | 0 repeticiones + ♦ + 7 | Envía **1x 7 de Diamantes** y desbloquea |
| `2101` | 2 repeticiones + ♥ + As | Envía **2x As de Corazones** pero muestra "PIN incorrecto" |
| `3405` | 3 repeticiones + ♦ + 5 | Envía **3x 5 de Diamantes** pero muestra "PIN incorrecto" |

### Comportamiento de la App

- **PINs que empiezan con 0**: Siempre desbloquean y envían 1 carta
- **PINs que empiezan con 1-9**: Muestran "PIN incorrecto" pero envían N cartas en segundo plano

Esto permite usar la pantalla de bloqueo de forma natural mientras se transmiten datos al ESP32.

## 🛠️ Hardware Necesario

- **ESP32-C3 MINI** con display OLED SSD1306 (128x64)
- **Smartphone Android** (API 24+, Android 7.0+)
- Conexión I2C para el display (SDA/SCL)

### Conexiones Display OLED

```
ESP32-C3      OLED SSD1306
--------      ------------
GPIO 8   →    SDA
GPIO 9   →    SCL
3.3V     →    VCC
GND      →    GND
```

## 📱 Instalación

### 1. ESP32

#### Requisitos
- Arduino IDE o PlatformIO
- Librerías necesarias:
  - `WiFi.h`
  - `WebServer.h`
  - `ArduinoJson` (versión 6.x)
  - `Adafruit_GFX`
  - `Adafruit_SSD1306`

#### Pasos

1. Abre el archivo `esp32/LockScreenCard/LockScreenCard.ino`

2. Instala las librerías desde el Library Manager de Arduino:
   ```
   - Adafruit GFX Library
   - Adafruit SSD1306
   - ArduinoJson
   ```

3. Configura el board:
   - Board: "ESP32C3 Dev Module"
   - Flash Size: 4MB
   - Partition Scheme: Default

4. Sube el código al ESP32

5. El ESP32 creará una red WiFi AP:
   - **SSID**: `ESP32-LockScreen`
   - **Password**: `12345678`
   - **IP**: `192.168.4.1`

### 2. Aplicación Android

#### Opción A: Compilar desde Android Studio

1. Abre Android Studio

2. Abre el proyecto desde la carpeta `android/`

3. Espera a que Gradle sincronice las dependencias

4. Conecta tu dispositivo Android o usa un emulador

5. Haz clic en **Run** ▶️

6. Instala la aplicación en tu dispositivo

#### Opción B: Instalar APK precompilado

Si tienes un APK ya compilado:

```bash
adb install -r LockScreenCard.apk
```

### 3. Permisos de Android

La aplicación necesita los siguientes permisos:

- ✅ **Internet**: Para conectarse al ESP32
- ✅ **Mostrar sobre otras apps**: Para simular una pantalla de bloqueo real
- ✅ **Deshabilitar pantalla de bloqueo**: Permite que la app se muestre al encender la pantalla

**Importante**: Después de instalar, ve a:
- `Configuración → Apps → Lock Screen → Permisos`
- Activa "Mostrar sobre otras aplicaciones"

## 🚀 Uso

1. **Enciende el ESP32**: Verás en el display la información del WiFi AP

2. **Conecta el móvil al WiFi del ESP32**:
   - WiFi: `ESP32-LockScreen`
   - Contraseña: `12345678`

3. **Abre la app Lock Screen** en tu Android

4. **Ingresa un PIN de 4 dígitos**:
   - La pantalla mostrará puntos conforme escribes
   - Al completar 4 dígitos, se procesará automáticamente

5. **Observa el display del ESP32**:
   - Verás la carta que enviaste con su símbolo y valor

### Ejemplo de Sesión

```
Paso 1: Conectar WiFi "ESP32-LockScreen"
Paso 2: Abrir app Lock Screen
Paso 3: Ingresar PIN "0101"
        → ESP32 muestra: "♥ Ace" (As de Corazones)
        → App se desbloquea ✅

Paso 4: Abrir app nuevamente
Paso 5: Ingresar PIN "3213"
        → ESP32 recibe 3x "Queen of Spades"
        → App muestra "PIN incorrecto" ❌
        → Pero los datos se enviaron en segundo plano
```

## 🔧 Configuración Avanzada

### Cambiar IP del ESP32

Edita en `MainActivity.kt` (línea ~204):

```kotlin
val esp32Ip = "192.168.4.1" // Cambia aquí
```

### Cambiar credenciales WiFi del AP

Edita en `LockScreenCard.ino` (líneas 18-19):

```cpp
const char* ssid = "ESP32-LockScreen";
const char* password = "12345678";
```

### Personalizar la apariencia de la pantalla de bloqueo

Edita los colores en `MainActivity.kt`:

```kotlin
MaterialTheme(
    colorScheme = darkColorScheme(
        primary = Color(0xFF6200EE),      // Color principal
        background = Color(0xFF121212),    // Fondo
        surface = Color(0xFF1E1E1E)       // Superficie
    ),
    content = content
)
```

## 📊 Estructura del Proyecto

```
HACKESP32-C3MINI/
├── android/                          # Aplicación Android
│   ├── app/
│   │   ├── src/main/
│   │   │   ├── AndroidManifest.xml
│   │   │   ├── java/com/hackesp32/lockscreen/
│   │   │   │   └── MainActivity.kt   # Lógica principal de la app
│   │   │   └── res/
│   │   │       └── values/
│   │   │           └── themes.xml
│   │   ├── build.gradle.kts
│   │   └── proguard-rules.pro
│   ├── build.gradle.kts
│   ├── settings.gradle.kts
│   └── gradle.properties
│
└── esp32/                            # Código ESP32
    └── LockScreenCard/
        └── LockScreenCard.ino        # Sketch Arduino
```

## 🔍 Depuración

### Ver logs del ESP32

Abre el Serial Monitor en Arduino IDE a **115200 baudios**:

```
Configurando Access Point...
AP IP address: 192.168.4.1
Servidor HTTP iniciado
Recibido: {"card":"Ace of Hearts"}
Carta recibida: Ace of Hearts
Carta mostrada en display: Ace of Hearts
```

### Ver logs de Android

```bash
adb logcat | grep -i "card"
```

### Probar el endpoint del ESP32

Con el móvil conectado al WiFi del ESP32:

```bash
curl -X POST http://192.168.4.1/card \
  -H "Content-Type: application/json" \
  -d '{"card":"King of Spades"}'
```

## 🎨 Características

- ✨ **Pantalla de bloqueo realista** con hora y fecha en tiempo real
- 🎯 **Codificación discreta** mediante PINs numéricos
- 📡 **Comunicación WiFi** directa sin necesidad de router
- 🃏 **Soporte para 52 cartas** (4 palos × 13 valores)
- 🔁 **Sistema de repetición** para enviar múltiples cartas
- 📺 **Display OLED** con símbolos de cartas
- 🌐 **Interfaz web** en el ESP32 para monitoreo

## 🔐 Seguridad y Discreción

- La app parece una pantalla de bloqueo normal
- No hay indicadores visuales de transmisión de datos
- Los PINs "incorrectos" no levantan sospechas
- El WiFi AP puede tener un nombre genérico
- Funciona sin conexión a Internet

## 🐛 Solución de Problemas

### La app no se conecta al ESP32

- ✅ Verifica que estés conectado al WiFi `ESP32-LockScreen`
- ✅ Comprueba que la IP sea `192.168.4.1`
- ✅ Asegúrate de que el ESP32 esté encendido

### El display no muestra nada

- ✅ Verifica las conexiones I2C (SDA pin 8, SCL pin 9)
- ✅ Comprueba la dirección del display (0x3C)
- ✅ Usa un scanner I2C para detectar el dispositivo

### La app muestra "Error"

- ✅ Revisa los logs de Android con `adb logcat`
- ✅ Verifica que el formato del PIN sea correcto
- ✅ Comprueba la conectividad WiFi

## 📝 Licencia

Este proyecto es de código abierto y está disponible para uso educativo y de investigación.

## 🤝 Contribuciones

Las contribuciones son bienvenidas. Por favor, abre un issue o pull request para mejoras.

---

**⚠️ Aviso**: Este proyecto es para fines educativos y de investigación. Úsalo de manera responsable y conforme a las leyes locales.
