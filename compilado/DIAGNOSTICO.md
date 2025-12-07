# 🔍 DIAGNÓSTICO DE PROBLEMAS DE CONEXIÓN

## ❌ PROBLEMA REPORTADO
- WiFi "Mm_wifi" aparece y el móvil se conecta
- Pero las apps no pueden enviar datos al ESP32
- Dan error al intentar comunicarse

## ✅ SOLUCIONES APLICADAS

### 1. **Aumentados los timeouts de conexión HTTP**
   - **Antes**: 10 segundos (por defecto)
   - **Ahora**: 10 segundos de conexión + 10 de escritura + 10 de lectura
   - **Razón**: Redes WiFi locales sin internet necesitan más tiempo

### 2. **Agregado logging detallado**
   - Ahora la app muestra logs en logcat con tag "ESP32"
   - Puedes ver exactamente qué está fallando
   - Para ver logs: `adb logcat -s ESP32`

### 3. **ArduinoJson actualizado a v7**
   - El ESP32 ahora usa la sintaxis correcta de ArduinoJson v7
   - Eliminados todos los warnings de compilación
   - Mejor manejo de JSON

### 4. **WiFi SSID cambiado a "Mm_wifi"**
   - Ahora coincide con lo que el usuario espera
   - Contraseña: 12345678

## 🔧 CÓMO DIAGNOSTICAR EL PROBLEMA

### Paso 1: Verificar el ESP32
1. Abre Arduino IDE
2. Abre Monitor Serial (115200 baudios)
3. Sube el código `esp32/UnifiedESP32.ino`
4. Deberías ver:
   ```
   Configurando Access Point...
   AP IP address: 192.168.4.1
   Servidor HTTP iniciado
   ```

### Paso 2: Verificar la red WiFi
1. Busca la red "Mm_wifi" en tu móvil
2. Conéctate (contraseña: 12345678)
3. Android dirá "Sin acceso a Internet" → **NORMAL, ignora**
4. Ve a Ajustes → WiFi → Mm_wifi → Avanzado
5. Verifica que la IP de tu móvil sea algo como `192.168.4.2`
6. La puerta de enlace debe ser `192.168.4.1`

### Paso 3: Probar la conexión desde navegador
1. Conectado a "Mm_wifi", abre un navegador
2. Ve a: `http://192.168.4.1`
3. Deberías ver una página web con:
   - "ESP32 Unified System"
   - Modo actual (CALCULATOR o LOCKSCREEN)
   - Últimas cartas/operaciones

Si NO ves la página web → **El problema es del ESP32**

Si SÍ ves la página web → **El problema es de la app Android**

### Paso 4: Probar endpoint /card manualmente
1. Desde un ordenador conectado a "Mm_wifi"
2. Usa curl o Postman:
   ```bash
   curl -X POST http://192.168.4.1/card \
     -H "Content-Type: application/json" \
     -d '{"card":"Ace of Hearts"}'
   ```
3. Deberías recibir: `{"status":"ok"}`
4. El ESP32 debería mostrar la carta en la pantalla OLED

### Paso 5: Ver logs de Android
1. Conecta el móvil por USB
2. Habilita Depuración USB en el móvil
3. Ejecuta: `adb logcat -s ESP32`
4. Abre la app e introduce un PIN
5. Verás exactamente qué error ocurre:
   - `Connection check: 200` → Conexión OK
   - `Response code: 200` → Envío exitoso
   - `Connection check failed: timeout` → Timeout
   - `Error sending card: 404` → Endpoint no encontrado

## 🐛 POSIBLES CAUSAS Y SOLUCIONES

### Error: "Connection timeout"
**Causa**: El móvil no puede alcanzar el ESP32
**Solución**:
- Verifica que estés conectado a "Mm_wifi"
- Verifica que el ESP32 esté encendido
- Reinicia el ESP32
- Desconecta y reconecta al WiFi

### Error: "404 Not Found"
**Causa**: El endpoint no existe en el ESP32
**Solución**:
- Verifica que el ESP32 tenga el código actualizado
- El endpoint debe ser `/card` (no `/cards` ni `/data`)
- Sube de nuevo el código al ESP32

### Error: "Failed to send card"
**Causa**: El servidor respondió pero con error
**Solución**:
- Verifica los logs del Monitor Serial del ESP32
- Puede ser un error de JSON
- Verifica que el JSON sea válido: `{"card":"nombre"}`

### Error: "UnknownHostException"
**Causa**: El móvil no puede resolver la IP 192.168.4.1
**Solución**:
- Verifica que estés conectado a "Mm_wifi"
- NO uses DNS, usa directamente la IP
- Verifica que Android no haya desconectado la red por "Sin Internet"

### Android desconecta automáticamente del WiFi
**Causa**: Android detecta que no hay internet y cambia a datos móviles
**Solución**:
1. Ajustes → WiFi → Mm_wifi → Avanzado
2. Desactiva "Cambiar a datos móviles automáticamente"
3. O desactiva datos móviles mientras usas la app

## 📊 TABLA DE DIAGNÓSTICO RÁPIDO

| Síntoma | Causa probable | Solución |
|---------|---------------|----------|
| No veo "Mm_wifi" | ESP32 no arrancó el AP | Sube el código de nuevo |
| Me conecto pero no navega | Normal (no hay internet) | Ignora, es esperado |
| App se cierra inmediato | PIN empieza con 0 | Normal si es 0XXX |
| App dice "Error" | PIN inválido | Usa formato XPVV correcto |
| Botón borrar siempre rojo | No conecta al ESP32 | Verifica conexión WiFi |
| Monitor Serial no muestra nada | Baudios incorrectos | Configura 115200 |

## 🎯 PRUEBA FINAL

Haz esta prueba para confirmar que todo funciona:

### Prueba 1: Conexión básica
1. ✅ ESP32 encendido y Monitor Serial muestra "AP IP address: 192.168.4.1"
2. ✅ Móvil conectado a "Mm_wifi"
3. ✅ Navegador muestra página web en http://192.168.4.1
4. ✅ Botón borrar de la app es AZUL (no rojo)

### Prueba 2: Envío de 1 carta
1. ✅ PIN: `0101` (As de Corazones)
2. ✅ Botón borrar se pone VERDE
3. ✅ App se cierra
4. ✅ ESP32 muestra en OLED: "Ace of Hearts"
5. ✅ Monitor Serial dice: "Recibido en /card: {"card":"Ace of Hearts"}"

### Prueba 3: Acumulación de 3 cartas
1. ✅ PIN: `3405` → Muestra "Pin erróneo", NO cierra
2. ✅ PIN: `3405` → Muestra "Pin erróneo", NO cierra
3. ✅ PIN: `3405` → Botón VERDE, envía 3 cartas, cierra app
4. ✅ ESP32 recibió 3 veces "5 of Diamonds"

Si **TODAS** las pruebas pasan → ✅ Sistema funcionando perfectamente

Si alguna falla → Revisa los logs de Android y Monitor Serial

## 💡 TIPS ADICIONALES

1. **Siempre** revisa el Monitor Serial del ESP32 primero
2. **Nunca** uses VPN mientras usas la app
3. **Desactiva** datos móviles para evitar que Android cambie de red
4. **Mantén** el móvil cerca del ESP32 (señal fuerte)
5. **Reinicia** el ESP32 si algo falla de forma extraña

## 📝 REGISTRO DE CAMBIOS

### Versión actual (2025-12-07):
- ✅ Timeouts aumentados a 10 segundos
- ✅ Logging detallado agregado
- ✅ ArduinoJson v7 compatible
- ✅ WiFi SSID cambiado a "Mm_wifi"
- ✅ Lógica de acumulación corregida
- ✅ Indicador visual de conexión

### Problemas conocidos resueltos:
- ❌ StaticJsonDocument deprecado → ✅ JsonDocument
- ❌ containsKey() deprecado → ✅ isNull()
- ❌ Timeouts muy cortos → ✅ 10 segundos
- ❌ Sin logs de debug → ✅ Logs detallados
- ❌ SSID "ESP32-Hidden" → ✅ "Mm_wifi"

---

**Si después de todo esto sigue sin funcionar**, proporciona:
1. Logs del Monitor Serial del ESP32
2. Logs de `adb logcat -s ESP32`
3. Capturas de pantalla del error en la app
