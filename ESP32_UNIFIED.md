# 🔄 ESP32 Unificado - Sistema Dual

## 📋 Descripción

El código **UnifiedESP32.ino** combina las funcionalidades de **Calculadora WiFi** y **Pantalla de Bloqueo** en un solo sistema ESP32. Puedes cambiar entre modos sin necesidad de reprogramar el dispositivo.

## 🎯 Características

### ✨ Modos de Operación

1. **🔢 MODO CALCULADORA**
   - Recibe operaciones matemáticas
   - Muestra cálculos en el display
   - Endpoints: `/calc` o `/data`

2. **🃏 MODO PANTALLA DE BLOQUEO**
   - Recibe cartas de póker
   - Muestra símbolos de palos (♥ ♠ ♣ ♦)
   - Endpoints: `/card` o `/data`

### 🔀 Cambio de Modo

**3 formas de cambiar de modo:**

#### 1️⃣ Botón Físico (BOOT)
- Presiona el botón **BOOT** del ESP32-C3 (GPIO9)
- El display mostrará el nuevo modo
- Debounce de 500ms para evitar cambios accidentales

#### 2️⃣ Interfaz Web
- Abre `http://192.168.4.1` en tu navegador
- Click en "Modo Calculadora" o "Modo Pantalla Bloqueo"
- Cambio instantáneo

#### 3️⃣ API REST
```bash
# Cambiar a modo calculadora
curl -X POST http://192.168.4.1/mode -d "mode=calculator"

# Cambiar a modo pantalla de bloqueo
curl -X POST http://192.168.4.1/mode -d "mode=lockscreen"

# Consultar modo actual
curl http://192.168.4.1/mode
```

## 📡 Endpoints API

### 1. `/data` (Universal - Autodetección)
Detecta automáticamente el tipo de datos:

**Calculadora:**
```bash
curl -X POST http://192.168.4.1/data \
  -H "Content-Type: application/json" \
  -d '{"operation":"5+3=8"}'
```

**Carta:**
```bash
curl -X POST http://192.168.4.1/data \
  -H "Content-Type: application/json" \
  -d '{"card":"Ace of Hearts"}'
```

### 2. `/calc` (Específico para Calculadora)
```bash
# Formato 1: Operación completa
curl -X POST http://192.168.4.1/calc \
  -H "Content-Type: application/json" \
  -d '{"operation":"12 × 5 = 60"}'

# Formato 2: Componentes separados
curl -X POST http://192.168.4.1/calc \
  -H "Content-Type: application/json" \
  -d '{"num1":"12","operator":"×","num2":"5","result":"60"}'
```

### 3. `/card` (Específico para Cartas)
```bash
curl -X POST http://192.168.4.1/card \
  -H "Content-Type: application/json" \
  -d '{"card":"Queen of Spades"}'
```

### 4. `/mode` (Control de Modo)
```bash
# GET: Consultar modo actual
curl http://192.168.4.1/mode
# Respuesta: {"mode":"calculator"} o {"mode":"lockscreen"}

# POST: Cambiar modo
curl -X POST http://192.168.4.1/mode -d "mode=lockscreen"
# Respuesta: {"status":"ok","mode":"lockscreen"}
```

### 5. `/` (Interfaz Web)
Abre `http://192.168.4.1` en un navegador para:
- Ver el modo actual
- Cambiar entre modos con botones
- Ver historial de operaciones/cartas

## 🔧 Configuración

### WiFi AP
```cpp
const char* ssid = "ESP32-Hidden";
const char* password = "12345678";
```

**IP por defecto:** `192.168.4.1`

### Display OLED

```
ESP32-C3      OLED SSD1306
--------      ------------
GPIO 8   →    SDA
GPIO 9   →    SCL
3.3V     →    VCC
GND      →    GND
```

### Librerías Necesarias
- `WiFi.h`
- `WebServer.h`
- `ArduinoJson` (v6.x)
- `Adafruit_GFX`
- `Adafruit_SSD1306`

## 🚀 Instalación

1. **Instalar librerías** en Arduino IDE:
   - Adafruit GFX Library
   - Adafruit SSD1306
   - ArduinoJson

2. **Configurar board**:
   - Board: "ESP32C3 Dev Module"
   - Flash Size: 4MB
   - Partition Scheme: Default

3. **Subir sketch**: `esp32/UnifiedESP32/UnifiedESP32.ino`

4. **Conectar display OLED** según el esquema anterior

## 🎮 Uso con las Apps Android

### Con Calculadora WiFi

1. Cambia el ESP32 a **MODO CALCULADORA**:
   - Presiona botón BOOT, o
   - Usa la web: `http://192.168.4.1`

2. **Modifica la app** para usar endpoint correcto:

En el código de la app calculadora, cambia:
```kotlin
val esp32Ip = "192.168.4.1"
val endpoint = "/calc" // o "/data"
```

3. Conecta al WiFi `ESP32-Hidden` y usa la app normalmente

### Con Pantalla de Bloqueo

1. Cambia el ESP32 a **MODO LOCKSCREEN**:
   - Presiona botón BOOT, o
   - Usa la web: `http://192.168.4.1`

2. **Modifica la app** si es necesario:

En `MainActivity.kt`, línea ~204:
```kotlin
val esp32Ip = "192.168.4.1"
// El endpoint ya es /card por defecto
```

3. Conecta al WiFi `ESP32-Hidden` y usa la app normalmente

## 📊 Visualización en Display

### Modo Calculadora
```
┌─────────────────┐
│ CALCULADORA     │
├─────────────────┤
│                 │
│   12 × 5        │
│   = 60          │
│                 │
└─────────────────┘
```

### Modo Pantalla de Bloqueo
```
┌─────────────────┐
│ CARTA RECIBIDA  │
├─────────────────┤
│                 │
│    ♥ Ace        │
│                 │
│ Ace of Hearts   │
└─────────────────┘
```

### Pantalla de Espera
```
┌─────────────────┐
│ CALC            │  o  │ LOCK            │
│                 │     │                 │
│ Modo:           │     │ Modo:           │
│ Calculadora     │     │ Pantalla Bloq.  │
│                 │     │                 │
│ Esperando...    │     │ Esperando...    │
└─────────────────┘     └─────────────────┘
```

## 🔍 Debugging

### Monitor Serial (115200 baudios)
```
Configurando Access Point...
AP IP address: 192.168.4.1
Servidor HTTP iniciado
Modo: LOCK SCREEN
Recibido en /card: {"card":"King of Spades"}
Carta: King of Spades
Carta mostrada: King of Spades
```

### Prueba de Endpoints
```bash
# Test calculadora
curl -X POST http://192.168.4.1/calc \
  -H "Content-Type: application/json" \
  -d '{"operation":"7+3=10"}'

# Test carta
curl -X POST http://192.168.4.1/card \
  -H "Content-Type: application/json" \
  -d '{"card":"Jack of Diamonds"}'

# Test universal (auto-detecta)
curl -X POST http://192.168.4.1/data \
  -H "Content-Type: application/json" \
  -d '{"card":"2 of Clubs"}'
```

## 💡 Ventajas del Sistema Unificado

✅ **Un solo sketch** para ambas funciones
✅ **Cambio de modo sin reprogramar**
✅ **Endpoint universal** con autodetección
✅ **Interfaz web** para control remoto
✅ **Historial** de últimas 5 operaciones/cartas
✅ **Mismo WiFi AP** para ambos modos
✅ **Menor consumo de memoria** que dos sketches separados

## 🔐 Compatibilidad

### Con App Calculadora
- ✅ Endpoint `/calc` compatible
- ✅ Endpoint `/data` con autodetección
- ⚙️ Cambiar SSID a `ESP32-Hidden` en la app

### Con App Lock Screen
- ✅ Endpoint `/card` compatible
- ✅ Endpoint `/data` con autodetección
- ⚙️ Cambiar SSID a `ESP32-Hidden` en la app

### Opcional: Mantener SSIDs Diferentes
Si prefieres SSIDs específicos por app, puedes:

```cpp
// Para calculadora
const char* ssid = "ESP32-Calculator";

// Para lock screen
const char* ssid = "ESP32-LockScreen";
```

Y cambiar manualmente según la app que uses. Pero con el sistema unificado **esto ya no es necesario** 🎉

## 📝 Resumen de Comandos

| Acción | Método | Endpoint | Body |
|--------|--------|----------|------|
| Ver modo actual | GET | `/mode` | - |
| Cambiar a calculadora | POST | `/mode` | `mode=calculator` |
| Cambiar a lockscreen | POST | `/mode` | `mode=lockscreen` |
| Enviar operación | POST | `/calc` | `{"operation":"5+3=8"}` |
| Enviar carta | POST | `/card` | `{"card":"Ace of Hearts"}` |
| Autodetección | POST | `/data` | JSON con `card` u `operation` |

---

**🎯 Con este sistema unificado, tienes máxima flexibilidad en un solo dispositivo!**
