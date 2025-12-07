#!/bin/bash

echo "===================================="
echo " COMPILADOR HACKESP32-C3MINI"
echo "===================================="
echo ""

echo "[1/3] Compilando app Android..."
cd android
./gradlew assembleDebug

if [ $? -eq 0 ]; then
    echo ""
    echo "[OK] Compilación exitosa!"
    echo ""
    echo "[2/3] APK generado en:"
    echo "android/app/build/outputs/apk/debug/app-debug.apk"
    echo ""
    echo "[3/3] Copiando APK a la carpeta principal..."
    cp app/build/outputs/apk/debug/app-debug.apk ../LockScreen.apk
    echo ""
    echo "===================================="
    echo " COMPILACIÓN COMPLETA"
    echo "===================================="
    echo ""
    echo "APK disponible en: compilado/LockScreen.apk"
    echo "Código ESP32 en: compilado/esp32/UnifiedESP32.ino"
    echo ""
else
    echo ""
    echo "[ERROR] La compilación falló"
    echo "Verifica que tengas Java JDK instalado"
    echo ""
fi

cd ..
