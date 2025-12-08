/*
 * ESP32-C3 Unified System
 * Compatible with:
 * 1. Calculadora WiFi App (web interface + beacon transmission)
 * 2. Lock Screen App (HTTP /card endpoint)
 */

#include <WiFi.h>
#include <WebServer.h>
#include "esp_wifi.h"
#include <DNSServer.h>
#include <ArduinoJson.h>

WebServer server(80);
DNSServer dnsServer;

const char* AP_SSID = "Mm_wifi";
const char* AP_PASSWORD = "12345678";
const char* DOMINIO = "magic.mazo";
int numCartas = 1;

// Transmisión de beacons
bool transmitiendo = false;

// Beacon Packet buffer
uint8_t packet[128] = {
  0x80, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x64, 0x00, 0x31, 0x04, 0x00
};

String cartasSeleccionadas[10];
char ssids[500][32];
int totalRedesActivas = 0;

// Caracteres invisibles que funcionan bien con emojis en Android
const char* invisibles[] = {
  "",
  "\u200B",
  "\u200B\u200B",
  "\u200C",
  "\u200C\u200C",
  "\u200D",
  "\u200D\u200D",
  "\u200B\u200C",
  "\u200B\u200D",
  "\u200C\u200B",
  "\u200C\u200D",
  "\u200D\u200B",
  "\u200D\u200C",
  "\u200B\u200B\u200B",
  "\u200C\u200C\u200C",
  "\u200D\u200D\u200D",
  "\u200B\u200C\u200B",
  "\u200B\u200D\u200B",
  "\u200C\u200B\u200C",
  "\u200C\u200D\u200C",
  "\u200D\u200B\u200D",
  "\u200D\u200C\u200D",
  "\u200B\u200C\u200D",
  "\u200B\u200D\u200C",
  "\u200C\u200B\u200D",
  "\u200C\u200D\u200B",
  "\u200D\u200B\u200C",
  "\u200D\u200C\u200B",
  "\u200B\u200B\u200C",
  "\u200B\u200B\u200D"
};

// =================== HTML ===================
// =================== HTML ===================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>ESP32 Card Transmitter</title>
<style>
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    padding: 20px;
  }
  .container {
    background: rgba(255,255,255,0.95);
    border-radius: 20px;
    box-shadow: 0 20px 60px rgba(0,0,0,0.3);
    max-width: 500px;
    width: 100%;
    padding: 30px;
    animation: fadeIn 0.5s ease-in;
  }
  @keyframes fadeIn {
    from { opacity: 0; transform: translateY(20px); }
    to { opacity: 1; transform: translateY(0); }
  }
  h1 {
    text-align: center;
    font-size: 32px;
    margin-bottom: 10px;
    background: linear-gradient(45deg, #667eea, #764ba2);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    font-weight: 800;
    letter-spacing: 2px;
  }
  .subtitle {
    text-align: center;
    color: #666;
    font-size: 14px;
    margin-bottom: 30px;
  }
  .status {
    padding: 15px;
    border-radius: 10px;
    text-align: center;
    font-weight: 600;
    margin-bottom: 25px;
    transition: all 0.3s ease;
  }
  .status.inactive {
    background: #fee;
    color: #c33;
    border: 2px solid #fcc;
  }
  .status.active {
    background: #efe;
    color: #3c3;
    border: 2px solid #cfc;
    animation: pulse 2s infinite;
  }
  @keyframes pulse {
    0%, 100% { opacity: 1; }
    50% { opacity: 0.8; }
  }
  .card-display {
    background: linear-gradient(135deg, #f5f7fa 0%, #c3cfe2 100%);
    padding: 20px;
    border-radius: 12px;
    text-align: center;
    margin-bottom: 25px;
    min-height: 60px;
    display: flex;
    align-items: center;
    justify-content: center;
    font-size: 24px;
    font-weight: bold;
    color: #333;
  }
  .suits {
    display: grid;
    grid-template-columns: repeat(4, 1fr);
    gap: 12px;
    margin-bottom: 20px;
  }
  .suit-btn {
    aspect-ratio: 1;
    border: none;
    border-radius: 12px;
    font-size: 48px;
    cursor: pointer;
    transition: all 0.2s ease;
    box-shadow: 0 4px 15px rgba(0,0,0,0.1);
  }
  .suit-btn:hover {
    transform: translateY(-5px);
    box-shadow: 0 8px 25px rgba(0,0,0,0.2);
  }
  .suit-btn:active {
    transform: scale(0.95);
  }
  .suit-btn.spade { background: linear-gradient(135deg, #2c3e50 0%, #34495e 100%); }
  .suit-btn.heart { background: linear-gradient(135deg, #e74c3c 0%, #c0392b 100%); }
  .suit-btn.diamond { background: linear-gradient(135deg, #e67e22 0%, #d35400 100%); }
  .suit-btn.club { background: linear-gradient(135deg, #16a085 0%, #117a65 100%); }
  .ranks {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(60px, 1fr));
    gap: 10px;
    margin-bottom: 20px;
    max-height: 0;
    overflow: hidden;
    transition: max-height 0.3s ease;
  }
  .ranks.show {
    max-height: 500px;
  }
  .rank-btn {
    aspect-ratio: 1;
    border: 2px solid #667eea;
    background: white;
    border-radius: 10px;
    font-size: 20px;
    font-weight: bold;
    color: #667eea;
    cursor: pointer;
    transition: all 0.2s ease;
  }
  .rank-btn:hover {
    background: #667eea;
    color: white;
    transform: scale(1.05);
  }
  .controls {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 12px;
    margin-top: 25px;
  }
  .ctrl-btn {
    padding: 15px;
    border: none;
    border-radius: 10px;
    font-size: 16px;
    font-weight: 600;
    cursor: pointer;
    transition: all 0.2s ease;
    text-transform: uppercase;
    letter-spacing: 1px;
  }
  .ctrl-btn.start {
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    color: white;
    box-shadow: 0 4px 15px rgba(102, 126, 234, 0.4);
  }
  .ctrl-btn.start:hover {
    box-shadow: 0 6px 20px rgba(102, 126, 234, 0.6);
    transform: translateY(-2px);
  }
  .ctrl-btn.stop {
    background: linear-gradient(135deg, #ee5a6f 0%, #f29263 100%);
    color: white;
    box-shadow: 0 4px 15px rgba(238, 90, 111, 0.4);
  }
  .ctrl-btn.stop:hover {
    box-shadow: 0 6px 20px rgba(238, 90, 111, 0.6);
    transform: translateY(-2px);
  }
  .footer {
    text-align: center;
    margin-top: 25px;
    padding-top: 20px;
    border-top: 1px solid #ddd;
    color: #999;
    font-size: 12px;
  }
  .footer strong {
    color: #667eea;
  }
</style>
<script>
let currentSuit = '';
let selectedCard = '';

if (window.location.hostname === '192.168.4.1') {
  window.location.href = 'http://magic.mazo';
}

function chooseSuit(suit, emoji){
  currentSuit = suit;
  selectedCard = '';
  document.getElementById('rankSection').classList.add('show');
  document.getElementById('cardDisplay').innerHTML = '¿Qué carta? ' + emoji;
}

function chooseRank(rank){
  selectedCard = rank + ' ' + currentSuit;
  document.getElementById('cardDisplay').innerHTML = '<strong>✓ ' + selectedCard + '</strong>';
  document.getElementById('rankSection').classList.remove('show');
}

function startTransmission(){
  if(!selectedCard){
    alert('⚠️ Selecciona una carta primero');
    return;
  }
  fetch('/startTransmission?cards=' + encodeURIComponent(selectedCard) + '&count=1')
    .then(response => response.text())
    .then(data => {
      document.getElementById('status').innerHTML = '📡 Transmitiendo: ' + selectedCard + ' (30 redes)';
      document.getElementById('status').className = 'status active';
    })
    .catch(err => {
      alert('❌ Error: ' + err.message);
    });
}

function stopTransmission(){
  fetch('/stopTransmission')
    .then(response => response.text())
    .then(data => {
      document.getElementById('status').innerHTML = '⏸️ Transmisión detenida';
      document.getElementById('status').className = 'status inactive';
      document.getElementById('cardDisplay').innerHTML = 'Selecciona una carta';
    });
}
</script>
</head>
<body>
  <div class="container">
    <h1>🎴 ESP32 TRANSMITTER</h1>
    <div class="subtitle">Card Broadcasting System</div>

    <div class="status inactive" id="status">
      ⏸️ Inactivo - Selecciona una carta
    </div>

    <div class="card-display" id="cardDisplay">
      Selecciona una carta
    </div>

    <div class="suits">
      <button class="suit-btn spade" onclick="chooseSuit('♠','♠')">♠</button>
      <button class="suit-btn heart" onclick="chooseSuit('♥','♥')">♥</button>
      <button class="suit-btn diamond" onclick="chooseSuit('♦','♦')">♦</button>
      <button class="suit-btn club" onclick="chooseSuit('♣','♣')">♣</button>
    </div>

    <div id="rankSection" class="ranks">
      <button class="rank-btn" onclick="chooseRank('A')">A</button>
      <button class="rank-btn" onclick="chooseRank('2')">2</button>
      <button class="rank-btn" onclick="chooseRank('3')">3</button>
      <button class="rank-btn" onclick="chooseRank('4')">4</button>
      <button class="rank-btn" onclick="chooseRank('5')">5</button>
      <button class="rank-btn" onclick="chooseRank('6')">6</button>
      <button class="rank-btn" onclick="chooseRank('7')">7</button>
      <button class="rank-btn" onclick="chooseRank('8')">8</button>
      <button class="rank-btn" onclick="chooseRank('9')">9</button>
      <button class="rank-btn" onclick="chooseRank('10')">10</button>
      <button class="rank-btn" onclick="chooseRank('J')">J</button>
      <button class="rank-btn" onclick="chooseRank('Q')">Q</button>
      <button class="rank-btn" onclick="chooseRank('K')">K</button>
    </div>

    <div class="controls">
      <button class="ctrl-btn start" onclick="startTransmission()">▶ Iniciar</button>
      <button class="ctrl-btn stop" onclick="stopTransmission()">⏹ Detener</button>
    </div>

    <div class="footer">
      Desarrollado por <strong>EliteMagic</strong><br>
      ESP32-C3 • WiFi AP: Mm_wifi
    </div>
  </div>
</body>
</html>
)rawliteral";

// =================== FUNCIONES ===================
String clampSSID(const String& s) {
  return (s.length() <= 32) ? s : s.substring(0, 32);
}

void limpiarRedesAnteriores() {
  for(int i = 0; i < 500; i++) {
    ssids[i][0] = '\0';
  }
  totalRedesActivas = 0;
  Serial.println("Redes anteriores limpiadas");
}

void generarVariacionesCarta(const String& baseName) {
  limpiarRedesAnteriores();
  for(int i = 0; i < 30; i++) {
    String nombre = baseName;
    nombre += invisibles[i % 30];
    if (nombre.length() > 32) {
      nombre = nombre.substring(0, 32);
    }
    nombre.toCharArray(ssids[i], 32);
  }
  totalRedesActivas = 30;
  Serial.println("Generadas 30 redes de: " + baseName);
}

void configurarMultiplesCartas(String cardsParam, int count) {
  limpiarRedesAnteriores();
  Serial.println("Procesando múltiples cartas: " + cardsParam);

  int startIndex = 0;
  int cardIndex = 0;

  for(int i = 0; i <= cardsParam.length() && cardIndex < count; i++) {
    if(i == cardsParam.length() || cardsParam.charAt(i) == '|') {
      String card = cardsParam.substring(startIndex, i);
      card.trim();

      if(card.length() > 0) {
        card.toCharArray(ssids[cardIndex], 32);
        cartasSeleccionadas[cardIndex] = card;
        cardIndex++;
      }
      startIndex = i + 1;
    }
  }
  totalRedesActivas = cardIndex;
  Serial.println("Total redes configuradas: " + String(cardIndex));
}

// =================== HTTP HANDLERS ===================
void handleRoot() {
  server.send(200, "text/html", index_html);
}

void handleStartTransmission() {
  Serial.println("=== /startTransmission LLAMADO ===");

  if (!server.hasArg("cards")) {
    Serial.println("ERROR: No hay parámetro 'cards'");
    server.send(400, "text/plain", "No cards parameter");
    return;
  }

  String cardsParam = server.arg("cards");
  int count = 1;

  if (server.hasArg("count")) {
    count = server.arg("count").toInt();
  }

  Serial.println("Cartas recibidas: " + cardsParam);
  Serial.println("Cantidad: " + String(count));

  if (count == 1) {
    // Una sola carta: generar 30 variaciones
    generarVariacionesCarta(cardsParam);
  } else {
    // Múltiples cartas: 1 red por cada carta
    configurarMultiplesCartas(cardsParam, count);
  }

  transmitiendo = true;

  Serial.println("✅ Transmisión iniciada: " + String(totalRedesActivas) + " redes WiFi");
  server.send(200, "text/plain", "OK - Transmitiendo: " + cardsParam);
}

void handleStopTransmission() {
  Serial.println("=== /stopTransmission LLAMADO ===");
  transmitiendo = false;
  limpiarRedesAnteriores();
  Serial.println("Transmisión detenida");
  server.send(200, "text/plain", "OK - Detenido");
}

// Endpoint para Lock Screen App (deprecated, usa /startTransmission)
void handleCard() {
  Serial.println("=== /card LLAMADO (Lock Screen App - deprecated) ===");
  Serial.println("⚠️ Usar /startTransmission en su lugar");

  if (!server.hasArg("plain")) {
    server.send(400, "text/plain", "No body");
    return;
  }

  String body = server.arg("plain");
  Serial.println("Body recibido: " + body);

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);

  if (error) {
    Serial.println("ERROR: JSON inválido");
    server.send(400, "text/plain", "Invalid JSON");
    return;
  }

  if (!doc["card"].isNull()) {
    String cardName = doc["card"].as<String>();
    Serial.println("Carta desde Lock Screen: " + cardName);

    // Generar 30 redes WiFi con el nombre de la carta
    generarVariacionesCarta(cardName);
    transmitiendo = true;

    Serial.println("✅ Transmitiendo carta: " + cardName);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    Serial.println("ERROR: No hay campo 'card'");
    server.send(400, "text/plain", "No card field");
  }
}

// =================== SETUP & LOOP ===================
void setup() {
  Serial.begin(115200);
  delay(100);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&cfg);
  esp_wifi_set_storage(WIFI_STORAGE_RAM);
  esp_wifi_set_mode(WIFI_MODE_AP);
  esp_wifi_start();

  Serial.println("==========================================");
  Serial.println("🎴 ESP32 UNIFICADO - ACTIVO 🎴");
  Serial.println("==========================================");
  Serial.println("📱 WiFi: " + String(AP_SSID));
  Serial.println("🔑 Pass: " + String(AP_PASSWORD));
  Serial.print("🌐 IP: ");
  Serial.println(WiFi.softAPIP());

  dnsServer.start(53, DOMINIO, WiFi.softAPIP());
  Serial.println("📡 DNS: " + String(DOMINIO));

  server.on("/", handleRoot);
  server.on("/startTransmission", handleStartTransmission);
  server.on("/stopTransmission", handleStopTransmission);
  server.on("/card", HTTP_POST, handleCard);
  server.onNotFound(handleRoot);

  server.begin();
  Serial.println("✅ Servidor HTTP iniciado");
  Serial.println("==========================================");
  Serial.println("Endpoints:");
  Serial.println("  /startTransmission - Calculadora WiFi");
  Serial.println("  /card - Lock Screen App");
  Serial.println("==========================================");
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();

  if (transmitiendo) {
    for(int i = 0; i < totalRedesActivas; i++) {
      // MAC aleatoria
      packet[10] = packet[16] = random(256);
      packet[11] = packet[17] = random(256);
      packet[12] = packet[18] = random(256);
      packet[13] = packet[19] = random(256);
      packet[14] = packet[20] = random(256);
      packet[15] = packet[21] = random(256);

      int ssidLen = strlen(ssids[i]);
      packet[37] = ssidLen;

      for(int j = 0; j < ssidLen; j++) {
        packet[38 + j] = ssids[i][j];
      }

      esp_wifi_80211_tx(WIFI_IF_AP, packet, 38 + ssidLen, false);
      delay(2);
    }
    delay(10);
  }
  delay(10);
}
