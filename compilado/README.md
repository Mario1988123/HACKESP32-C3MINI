# 🚀 HACKESP32-C3MINI - Carpeta de Compilación

## 📦 Contenido

Esta carpeta contiene todo lo necesario para compilar y usar el sistema HACKESP32-C3MINI:

```
compilado/
├── 📱 android/              # Proyecto completo Android (Pantalla de Bloqueo)
├── 🔧 esp32/               # Código ESP32-C3 (UnifiedESP32.ino)
├── 📜 compilar.bat         # Script compilación Windows
├── 📜 compilar.sh          # Script compilación Linux/Mac
├── 📖 INSTRUCCIONES.md     # Guía completa de uso
├── 🔍 DIAGNOSTICO.md       # Guía de solución de problemas
└── 📋 README.md            # Este archivo
```

## ⚡ INICIO RÁPIDO

### 🪟 Windows:
```cmd
cd compilado
compilar.bat
```

### 🐧 Linux / 🍎 Mac:
```bash
cd compilado
./compilar.sh
```

### 📱 Android Studio:
1. Abre Android Studio
2. File → Open → `compilado/android/`
3. Build → Build APK

## 📚 DOCUMENTACIÓN

- **[INSTRUCCIONES.md](INSTRUCCIONES.md)** - Guía completa paso a paso
- **[DIAGNOSTICO.md](DIAGNOSTICO.md)** - Solución de problemas de conexión

## 🎯 RESUMEN DEL SISTEMA

### Componentes:
1. **ESP32-C3** - Crea red WiFi "Mm_wifi" (192.168.4.1)
2. **App Android** - Pantalla de bloqueo que envía cartas codificadas en PINs

### Formato PIN (4 dígitos XPVV):
- **X** = Repeticiones (0 = enviar 1, 1-9 = acumular N)
- **P** = Palo (1=♥, 2=♠, 3=♣, 4=♦)
- **VV** = Valor (01=As, 02-10, 11=J, 12=Q, 13=K)

### Ejemplos:
- `0101` → Envía "Ace of Hearts" y cierra
- `3405` → Acumula 3x "5 of Diamonds"

## 🔧 CAMBIOS APLICADOS

### ✅ ESP32:
- WiFi SSID: "Mm_wifi"
- ArduinoJson v7 compatible
- Endpoints: /card, /calc, /data

### ✅ Android:
- Timeouts largos (10 seg)
- Logging detallado
- Indicador de conexión (botón borrar: 🔴🔵🟢)
- Lógica de acumulación corregida

## 🆘 AYUDA

¿Problemas? Revisa en orden:
1. [DIAGNOSTICO.md](DIAGNOSTICO.md) - Problemas comunes
2. [INSTRUCCIONES.md](INSTRUCCIONES.md) - Guía detallada
3. Monitor Serial del ESP32 (115200 baudios)
4. Logs Android: `adb logcat -s ESP32`

## 📊 ESTADO DEL PROYECTO

- ✅ Código ESP32 actualizado y probado
- ✅ App Android compilable
- ✅ Comunicación HTTP optimizada
- ✅ Documentación completa

---

**¡Todo listo para compilar y usar!** 🎉
