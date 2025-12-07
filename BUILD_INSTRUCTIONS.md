# 🔨 Instrucciones de Compilación

## 📱 Compilar APK Android

### Opción 1: GitHub Actions (Automático) ⭐ Recomendado

1. **Push tu código** a GitHub
2. Ve a la pestaña **Actions** en GitHub
3. Selecciona el workflow **"Build Android APK"**
4. Espera a que termine la compilación (≈5 minutos)
5. Descarga los artefactos:
   - `app-debug.apk` - Para pruebas
   - `app-release-unsigned.apk` - Para distribución (necesita firma)

**Ventajas:**
- ✅ No necesitas instalar nada en tu PC
- ✅ Compilación consistente
- ✅ Se ejecuta automáticamente en cada push
- ✅ Genera APK para debug y release

### Opción 2: Android Studio (Local)

#### Requisitos
- Android Studio Arctic Fox o superior
- JDK 17
- Android SDK (API 24-34)

#### Pasos

1. **Abrir proyecto**
   ```bash
   # Abre Android Studio
   # File → Open → Selecciona la carpeta "android/"
   ```

2. **Esperar sincronización de Gradle**
   - Android Studio descargará automáticamente las dependencias
   - Puede tardar 2-5 minutos la primera vez

3. **Compilar**
   - Click en **Build → Build Bundle(s) / APK(s) → Build APK(s)**
   - O usa el atajo: `Ctrl+F9` (Windows/Linux) o `Cmd+F9` (Mac)

4. **Encontrar el APK**
   ```
   android/app/build/outputs/apk/debug/app-debug.apk
   android/app/build/outputs/apk/release/app-release-unsigned.apk
   ```

### Opción 3: Línea de Comandos (Gradle)

#### Requisitos
- JDK 17
- Android SDK instalado
- Variable `ANDROID_HOME` configurada

#### Pasos

1. **Navegar al directorio**
   ```bash
   cd android/
   ```

2. **Compilar Debug APK**
   ```bash
   # Linux/Mac
   ./gradlew assembleDebug

   # Windows
   gradlew.bat assembleDebug
   ```

3. **Compilar Release APK**
   ```bash
   # Linux/Mac
   ./gradlew assembleRelease

   # Windows
   gradlew.bat assembleRelease
   ```

4. **Limpiar build**
   ```bash
   ./gradlew clean
   ```

5. **APK generado en:**
   ```
   app/build/outputs/apk/debug/app-debug.apk
   app/build/outputs/apk/release/app-release-unsigned.apk
   ```

### Opción 4: Generar Gradle Wrapper (Si no existe)

Si el `gradlew` no funciona o falta el wrapper:

```bash
cd android/

# Generar wrapper
gradle wrapper --gradle-version 8.0

# Verificar
./gradlew --version
```

## 🔐 Firmar APK para Producción

### Crear Keystore

```bash
keytool -genkey -v -keystore my-release-key.jks \
  -keyalg RSA -keysize 2048 -validity 10000 \
  -alias my-key-alias
```

### Configurar firma en build.gradle.kts

```kotlin
android {
    signingConfigs {
        create("release") {
            storeFile = file("my-release-key.jks")
            storePassword = "tu_password"
            keyAlias = "my-key-alias"
            keyPassword = "tu_password"
        }
    }

    buildTypes {
        release {
            signingConfig = signingConfigs.getByName("release")
            isMinifyEnabled = true
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }
}
```

### Compilar APK firmado

```bash
./gradlew assembleRelease
```

## 📦 ESP32

### Arduino IDE

1. **Instalar librerías:**
   - `Adafruit GFX Library`
   - `Adafruit SSD1306`
   - `ArduinoJson` (v6.x)

2. **Configurar board:**
   - Tools → Board → ESP32 Arduino → **ESP32C3 Dev Module**
   - Tools → Flash Size → **4MB**
   - Tools → Partition Scheme → **Default 4MB with spiffs**

3. **Abrir sketch:**
   - Para sistema unificado: `esp32/UnifiedESP32/UnifiedESP32.ino`
   - Para lock screen solo: `esp32/LockScreenCard/LockScreenCard.ino`

4. **Subir código:**
   - Click en **Upload** (→)
   - Presiona BOOT button si es necesario

### PlatformIO

#### platformio.ini

```ini
[env:esp32-c3-devkitm-1]
platform = espressif32
board = esp32-c3-devkitm-1
framework = arduino

lib_deps =
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit SSD1306@^2.5.9
    bblanchon/ArduinoJson@^6.21.5

monitor_speed = 115200
```

#### Compilar y subir

```bash
# Compilar
pio run

# Subir
pio run --target upload

# Monitor serial
pio device monitor
```

## 🐛 Solución de Problemas

### Android

**Error: SDK not found**
```bash
# Crear local.properties
echo "sdk.dir=/path/to/Android/sdk" > android/local.properties
```

**Error: Gradle sync failed**
```bash
cd android
./gradlew clean
./gradlew --refresh-dependencies
```

**Error: Java version mismatch**
```bash
# Instalar JDK 17
# Ubuntu/Debian
sudo apt install openjdk-17-jdk

# Mac (Homebrew)
brew install openjdk@17
```

### ESP32

**Error: Board not found**
- Instalar ESP32 board definitions:
  - File → Preferences → Additional Boards Manager URLs:
  - `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`

**Error: Display not working**
- Verificar conexiones I2C (SDA pin 8, SCL pin 9)
- Probar con I2C scanner
- Verificar dirección 0x3C o 0x3D

**Error: Upload failed**
- Presionar y mantener BOOT button durante upload
- Verificar puerto serial correcto
- Probar con velocidad de upload menor (115200)

## 📋 Checklist de Compilación

### Android
- [ ] JDK 17 instalado
- [ ] Android SDK instalado
- [ ] Variables de entorno configuradas
- [ ] Gradle wrapper presente
- [ ] Dependencias sincronizadas
- [ ] APK compilado exitosamente
- [ ] APK instalado y probado

### ESP32
- [ ] Arduino IDE/PlatformIO instalado
- [ ] ESP32 board definitions instalados
- [ ] Librerías instaladas
- [ ] Display OLED conectado
- [ ] Código compilado sin errores
- [ ] Código subido al ESP32
- [ ] WiFi AP funcionando
- [ ] Display mostrando información

## 🚀 Despliegue Rápido

### Setup Completo en 5 Pasos

```bash
# 1. Clonar repo
git clone <repo-url>
cd HACKESP32-C3MINI

# 2. Compilar Android (GitHub Actions)
git push origin main
# Esperar y descargar APK de Actions

# 3. Instalar APK en móvil
adb install app-debug.apk

# 4. Flashear ESP32
# Abrir Arduino IDE → Abrir sketch → Upload

# 5. Conectar y usar
# Conectar móvil a WiFi "ESP32-Hidden"
# Abrir app y usar
```

---

**¿Problemas?** Abre un issue en GitHub con:
- Versión de Android Studio / Arduino IDE
- Logs de error completos
- Modelo de ESP32
- Sistema operativo
