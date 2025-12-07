# 📦 HACKESP32-C3MINI - Compilación y Uso

## 🎯 Contenido de esta carpeta

```
compilado/
├── android/          # Proyecto completo de Android
├── esp32/           # Código del ESP32 (.ino)
├── compilar.bat     # Script para Windows
├── compilar.sh      # Script para Linux/Mac
└── INSTRUCCIONES.md # Este archivo
```

---

## 📱 1. COMPILAR LA APP ANDROID

### Windows:
1. Abre CMD o PowerShell en esta carpeta
2. Ejecuta:
   ```cmd
   cd android
   gradlew.bat assembleDebug
   ```
3. El APK estará en: `android/app/build/outputs/apk/debug/app-debug.apk`

### Linux/Mac:
1. Abre Terminal en esta carpeta
2. Ejecuta:
   ```bash
   cd android
   ./gradlew assembleDebug
   ```
3. El APK estará en: `android/app/build/outputs/apk/debug/app-debug.apk`

### Android Studio:
1. Abre Android Studio
2. File → Open → Selecciona la carpeta `android/`
3. Espera a que sincronice Gradle
4. Build → Build Bundle(s) / APK(s) → Build APK(s)
5. El APK estará en `app/build/outputs/apk/debug/`

---

## 🔧 2. SUBIR CÓDIGO AL ESP32

### Paso 1: Abrir en Arduino IDE
1. Abre Arduino IDE
2. File → Open → Selecciona `esp32/UnifiedESP32.ino`

### Paso 2: Configurar el IDE
1. **Placa**: Herramientas → Placa → ESP32 Arduino → ESP32C3 Dev Module
2. **Puerto**: Herramientas → Puerto → (Selecciona tu puerto COM/ttyUSB)

### Paso 3: Instalar librerías (si no las tienes)
Ve a Sketch → Include Library → Manage Libraries y busca:
- ✅ **ArduinoJson** (v7.0.0 o superior)
- ✅ **Adafruit GFX Library**
- ✅ **Adafruit SSD1306**

### Paso 4: Compilar y subir
1. Haz clic en el botón "Upload" (→)
2. Espera a que compile y suba
3. Abre Monitor Serial (115200 baudios)

### Paso 5: Verificar funcionamiento
Deberías ver en el monitor serial:
```
Configurando Access Point...
AP IP address: 192.168.4.1
Servidor HTTP iniciado
```

Y en la pantalla OLED del ESP32:
```
WiFi AP Activo
SSID: Mm_wifi
Pass: 12345678
IP: 192.168.4.1
```

---

## 📡 3. USAR LA APP

### Conectar al ESP32:
1. En tu móvil Android, ve a **Ajustes → WiFi**
2. Busca la red: **Mm_wifi**
3. Conéctate con la contraseña: **12345678**
4. ⚠️ **IMPORTANTE**: Android te dirá "Sin acceso a Internet" → Acepta y mantén conectado

### Instalar la APK:
1. Copia `app-debug.apk` a tu móvil
2. Abre el archivo APK
3. Acepta la instalación desde orígenes desconocidos
4. Instala la app

### Usar la pantalla de bloqueo:
1. Abre la app "Lock Screen"
2. Introduce un PIN de 4 dígitos en formato **XPVV**:
   - **X** = Repeticiones (0 = enviar 1 carta y cerrar, 1-9 = acumular N cartas)
   - **P** = Palo (1=♥ Corazones, 2=♠ Picas, 3=♣ Tréboles, 4=♦ Diamantes)
   - **VV** = Valor (01-13: As=01, 2-10=02-10, J=11, Q=12, K=13)

### Ejemplos de PINs:

#### Envío inmediato (empieza con 0):
- **0105** → Envía 1 carta: "Ace of Hearts" y cierra app
- **0213** → Envía 1 carta: "King of Spades" y cierra app
- **0307** → Envía 1 carta: "7 of Clubs" y cierra app

#### Acumulación (empieza con 1-9):
- **3405** → Guarda "5 of Diamonds" → Muestra "Pin erróneo" → NO cierra
- **3405** (2ª vez) → Guarda otra "5 of Diamonds" → Muestra "Pin erróneo" → NO cierra
- **3405** (3ª vez) → Envía las 3 cartas al ESP32 → Cierra app

#### Indicador de conexión (botón borrar):
- 🔴 **Rojo** = Desconectado del ESP32
- 🔵 **Azul** = Conectado al ESP32
- 🟢 **Verde** = Cartas enviadas correctamente

---

## 🐛 4. SOLUCIÓN DE PROBLEMAS

### ❌ No veo la red "Mm_wifi"
- ✅ Verifica que el ESP32 esté encendido
- ✅ Verifica que subiste el código correctamente
- ✅ Abre el Monitor Serial (115200 baudios) y verifica que diga "AP IP address: 192.168.4.1"
- ✅ Reinicia el ESP32 (botón RESET)

### ❌ La app se cierra inmediatamente
- ✅ Verifica que estés conectado a la red "Mm_wifi"
- ✅ No uses PINs que empiecen con 0 si quieres probar acumulación
- ✅ Verifica que el ESP32 esté respondiendo en http://192.168.4.1

### ❌ El ESP32 no muestra nada en la pantalla OLED
- ✅ Verifica que la pantalla OLED esté conectada correctamente
- ✅ Verifica la dirección I2C (0x3C por defecto)
- ✅ Instala las librerías Adafruit GFX y SSD1306

### ❌ Error "ArduinoJson.h: No such file or directory"
- ✅ Instala la librería ArduinoJson desde el gestor de librerías
- ✅ Usa ArduinoJson v7.0.0 o superior

### ❌ La app dice "Error" al introducir PIN
- ✅ Verifica que el PIN tenga 4 dígitos
- ✅ El palo debe ser 1-4
- ✅ El valor debe ser 01-13 (con cero delante si es menor que 10)
- ✅ Ejemplos válidos: 0101, 0213, 3405
- ✅ Ejemplos inválidos: 0500 (palo 5), 0115 (valor 15)

---

## 📋 5. CAMBIOS REALIZADOS

### ✅ ESP32 (UnifiedESP32.ino):
- Cambiado SSID a "Mm_wifi"
- Actualizado a ArduinoJson v7 (eliminados todos los warnings)
- Soporta endpoints: `/card`, `/calc`, `/data`
- Modo dual: Calculadora y Pantalla de Bloqueo

### ✅ Android (MainActivity.kt):
- Agregados timeouts largos para conexiones locales (10 segundos)
- Agregado logging detallado para debug
- Lógica de acumulación de cartas corregida
- Indicador visual de conexión (botón borrar con colores)
- Vibración y animación de shake en errores
- Formato de fecha en español
- Letras ABC en el teclado numérico
- Menú oculto (long press en el reloj) para fondo de pantalla

---

## 🎮 6. FUNCIONES AVANZADAS

### Menú oculto (App Android):
1. Mantén presionado el reloj por 2 segundos
2. Se abrirá un menú con opciones:
   - Seleccionar foto de fondo
   - Ajustes (próximamente)

### Cambiar modo del ESP32:
1. Presiona el botón BOOT (GPIO9) en el ESP32
2. El modo alternará entre:
   - 🔢 CALCULATOR MODE
   - 🔐 LOCKSCREEN MODE

### Ver estado del ESP32:
1. Abre un navegador web
2. Ve a: http://192.168.4.1
3. Verás el estado actual y últimas cartas/operaciones recibidas

---

## 📞 CONTACTO Y SOPORTE

Si tienes problemas:
1. Verifica los logs en el Monitor Serial del Arduino IDE
2. Verifica los logs de Android en logcat (busca tag "ESP32")
3. Asegúrate de estar usando ArduinoJson v7.x

---

## 🎉 ¡LISTO!

Ahora deberías tener:
- ✅ ESP32 transmitiendo WiFi "Mm_wifi"
- ✅ App Android instalada
- ✅ Comunicación funcionando entre ambos

¡Disfruta de tu sistema de pantalla de bloqueo con cartas! 🃏
