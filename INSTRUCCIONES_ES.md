# 🇪🇸 Instrucciones Rápidas

## Sistema de Cartas Oculto

Esta aplicación simula una pantalla de bloqueo pero en realidad envía cartas de póker al ESP32.

### 🎯 Cómo Funciona el PIN

**Formato: X-P-VV**

#### Primer Dígito (X): Repeticiones
- `0` → Enviar 1 vez y desbloquear
- `1-9` → Enviar N veces (muestra PIN incorrecto)

#### Segundo Dígito (P): Palo
- `1` → ♥ Corazones
- `2` → ♠ Picas
- `3` → ♣ Tréboles
- `4` → ♦ Diamantes

#### Últimos 2 Dígitos (VV): Valor
- `01` → As
- `02-10` → Números
- `11` → J (Jota)
- `12` → Q (Reina)
- `13` → K (Rey)

### 📝 Ejemplos Prácticos

#### Modo Desbloqueo (empieza con 0)
```
0101 → As de Corazones (1 vez) ✅ DESBLOQUEA
0213 → Rey de Picas (1 vez) ✅ DESBLOQUEA
0312 → Reina de Tréboles (1 vez) ✅ DESBLOQUEA
0407 → 7 de Diamantes (1 vez) ✅ DESBLOQUEA
```

#### Modo Oculto (empieza con 1-9)
```
2101 → As de Corazones (2 veces) ❌ Muestra "PIN incorrecto"
3405 → 5 de Diamantes (3 veces) ❌ Muestra "PIN incorrecto"
5213 → Rey de Picas (5 veces) ❌ Muestra "PIN incorrecto"
```

En el modo oculto, la app muestra error PERO envía las cartas en segundo plano.

### 🔧 Configuración Rápida

1. **Sube el código al ESP32**
   - Archivo: `esp32/LockScreenCard/LockScreenCard.ino`
   - Librerías: WiFi, WebServer, ArduinoJson, Adafruit_SSD1306

2. **Conecta el display OLED**
   ```
   ESP32 GPIO 8  →  SDA
   ESP32 GPIO 9  →  SCL
   3.3V          →  VCC
   GND           →  GND
   ```

3. **Compila e instala la app Android**
   - Abre carpeta `android/` en Android Studio
   - Click en Run ▶️

4. **Conecta el móvil al WiFi del ESP32**
   - SSID: `ESP32-LockScreen`
   - Password: `12345678`

5. **¡Listo para usar!**

### 💡 Consejos de Uso

✅ **Para desbloquear normalmente**: Usa PINs que empiecen con `0`

✅ **Para enviar datos discretamente**: Usa PINs que empiecen con `1-9`

✅ **Parecer natural**: Alterna entre ambos modos

✅ **Repetir cartas**: El primer dígito indica cuántas veces

### 🎭 Apariencia

La app se ve exactamente como una pantalla de bloqueo real:
- ⏰ Hora y fecha actualizadas
- 🔘 Teclado numérico
- 🔒 Puntos que se llenan al escribir
- ✨ Animaciones suaves

¡Nadie sospechará que está enviando datos al ESP32!

### 📡 Cómo Se Envían los Datos

1. Escribes el PIN en la pantalla de bloqueo
2. La app interpreta el código
3. Se envía HTTP POST al ESP32 (IP: 192.168.4.1)
4. El ESP32 recibe y muestra la carta en el display
5. La app se desbloquea (o muestra error según el modo)

### 🎮 Tabla de Referencia Rápida

| Valor | PIN Final |
|-------|-----------|
| As    | XX01      |
| 2-10  | XX02-XX10 |
| J     | XX11      |
| Q     | XX12      |
| K     | XX13      |

| Palo      | Posición 2 |
|-----------|------------|
| Corazones | X1XX       |
| Picas     | X2XX       |
| Tréboles  | X3XX       |
| Diamantes | X4XX       |

### 🚨 Importante

- El móvil DEBE estar conectado al WiFi del ESP32
- La IP por defecto es `192.168.4.1`
- Los valores van del 01 al 13 (dos dígitos siempre)
- Los palos van del 1 al 4 (un dígito)

¡Disfruta del sistema! 🎉
