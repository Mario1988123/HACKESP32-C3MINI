@echo off
echo ====================================
echo  COMPILADOR HACKESP32-C3MINI
echo ====================================
echo.

echo [1/3] Compilando app Android...
cd android
call gradlew.bat assembleDebug

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [OK] Compilacion exitosa!
    echo.
    echo [2/3] APK generado en:
    echo android\app\build\outputs\apk\debug\app-debug.apk
    echo.
    echo [3/3] Copiando APK a la carpeta principal...
    copy app\build\outputs\apk\debug\app-debug.apk ..\LockScreen.apk
    echo.
    echo ====================================
    echo  COMPILACION COMPLETA
    echo ====================================
    echo.
    echo APK disponible en: compilado\LockScreen.apk
    echo Codigo ESP32 en: compilado\esp32\UnifiedESP32.ino
    echo.
) else (
    echo.
    echo [ERROR] La compilacion fallo
    echo Verifica que tengas Java JDK instalado
    echo.
)

cd ..
pause
