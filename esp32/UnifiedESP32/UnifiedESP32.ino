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

// Caracteres invisibles para variaciones
const char* invisibles[] = {
  "\u200B", "\u2060", "\u200C", "\uFEFF", "\u200D",
  "\u200B\u2060", "\u200C\uFEFF", "\u200D\u200B", "\u2060\u200C", "\uFEFF\u200D",
  "\u200B\u200C", "\u200D\u2060", "\uFEFF\u200B", "\u200C\u200D", "\u2060\uFEFF",
  "\u200B\u200D", "\u200C\u2060", "\u200D\uFEFF", "\u2060\u200B", "\uFEFF\u200C",
  "\u200B\uFEFF", "\u200C\u200B", "\u200D\u200C", "\u2060\u200D", "\uFEFF\u2060",
  "\u200B\u200C\u200D", "\u200D\u2060\uFEFF", "\u2060\uFEFF\u200B", "\uFEFF\u200B\u200C", "\u200B\u200D\u2060"
};

// =================== HTML ===================
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>MOE HACK</title>
<style>
  body {
    margin:0; padding:0; text-align:center; font-family:Arial, Helvetica, sans-serif; color:#222;
    background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
    min-height: 100vh;
  }
  .wrap {
    background: rgba(255,255,255,0.95);
    margin: 10px auto;
    padding: 12px;
    border-radius: 12px;
    max-width: 980px;
    box-shadow: 0 8px 32px rgba(0,0,0,0.1);
  }
  h1 {
    font-size: 6vw;
    margin: 8px 0 6px;
    letter-spacing: 2px;
    background: linear-gradient(45deg, #ff6b6b, #4ecdc4);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
  }
  .status {
    padding: 8px;
    margin: 8px 0;
    border-radius: 6px;
    font-weight: bold;
    background: #f8f9fa;
    border: 2px solid #dee2e6;
    font-size: 3.5vw;
  }
  .status-active { background: #d4edda; color: #155724; border-color: #c3e6cb; }
  .status-inactive { background: #f8d7da; color: #721c24; border-color: #f5c6cb; }
  .suits {
    display:flex;
    justify-content:space-around;
    align-items:center;
    margin: 8px auto;
    max-width: 95%;
    gap:8px;
  }
  .suits button {
    flex:1;
    font-size:12vw;
    line-height:1;
    aspect-ratio:1/1;
    border:none;
    border-radius:12px;
    background:#fff;
    cursor:pointer;
    box-shadow:0 2px 8px rgba(0,0,0,.15);
    transition: transform 0.2s, box-shadow 0.2s;
  }
  .suits button:hover {
    transform: translateY(-2px);
    box-shadow:0 4px 12px rgba(0,0,0,.2);
  }
  .suits button:active { transform: scale(.97); }
  .spade   { color:black; border:3px solid black; }
  .heart   { color:red;   border:3px solid red; }
  .diamond { color:red;   border:3px solid red; }
  .club    { color:black; border:3px solid black; }
  .ranks {
    display:flex;
    flex-wrap:wrap;
    justify-content:center;
    gap:6px;
    max-width: 96%;
    margin: 10px auto;
  }
  .ranks button {
    width:15vw;
    max-width:70px;
    height:15vw;
    max-height:70px;
    font-size:5vw;
    border:2px solid #555;
    border-radius:8px;
    background:#eee;
    cursor:pointer;
    transition: background 0.2s;
  }
  .ranks button:active { background:#ddd; }
  #ranksSection { display:none; }
  .control-buttons {
    display: flex;
    justify-content: center;
    gap: 12px;
    margin: 12px 0;
  }
  .ctrl-btn {
    padding: 10px 20px;
    border: none;
    border-radius: 8px;
    color: white;
    font-size: 4vw;
    cursor: pointer;
    box-shadow: 0 2px 6px rgba(0,0,0,0.2);
    transition: transform 0.2s;
  }
  .ctrl-btn:active { transform: scale(0.95); }
  .start-btn { background: #28a745; }
  .stop-btn { background: #dc3545; }
  @media (min-width: 640px) {
    h1 { font-size: 38px; }
    .suits button { font-size: 80px; }
    .ranks button { font-size: 22px; width: 60px; height: 60px; }
    .ctrl-btn { font-size: 18px; }
    .status { font-size: 16px; }
  }
</style>
<script>
let currentSuit = '';
let selectedCard = '';

if (window.location.hostname === '192.168.4.1') {
  window.location.href = 'http://magic.mazo';
}

function chooseSuit(suit){
  currentSuit = suit;
  document.getElementById('ranksSection').style.display = 'flex';
}

function chooseRank(rank){
  selectedCard = rank + ' ' + currentSuit;
  document.getElementById('selectedCard').innerHTML = 'Carta: <strong>' + selectedCard + '</strong>';
  document.getElementById('ranksSection').style.display = 'none';
}

function startTransmission(){
  if(!selectedCard){
    alert('Selecciona una carta primero');
    return;
  }
  fetch('/startTransmission?cards=' + encodeURIComponent(selectedCard) + '&count=1')
    .then(response => response.text())
    .then(data => {
      document.getElementById('status').innerHTML = '🟢 TRANSMITIENDO: ' + selectedCard + ' (30 redes)';
      document.getElementById('status').className = 'status status-active';
    });
}

function stopTransmission(){
  fetch('/stopTransmission')
    .then(response => response.text())
    .then(data => {
      document.getElementById('status').innerHTML = '🔴 TRANSMISIÓN DETENIDA';
      document.getElementById('status').className = 'status status-inactive';
    });
}
</script>
</head>
<body>
  <div class="wrap">
    <h1>MOE HACK</h1>
    <div class="status status-inactive" id="status">🔴 INACTIVO - Selecciona carta y pulsa INICIAR</div>
    <div id="selectedCard" style="padding: 10px; font-size: 18px;">Selecciona una carta</div>
    <div class="suits">
      <button class="spade"   onclick="chooseSuit('♠')">&spades;</button>
      <button class="heart"   onclick="chooseSuit('♥')">&hearts;</button>
      <button class="diamond" onclick="chooseSuit('♦')">&diams;</button>
      <button class="club"    onclick="chooseSuit('♣')">&clubs;</button>
    </div>
    <div id="ranksSection" class="ranks">
      <button onclick="chooseRank('A')">A</button><button onclick="chooseRank('2')">2</button>
      <button onclick="chooseRank('3')">3</button><button onclick="chooseRank('4')">4</button>
      <button onclick="chooseRank('5')">5</button><button onclick="chooseRank('6')">6</button>
      <button onclick="chooseRank('7')">7</button><button onclick="chooseRank('8')">8</button>
      <button onclick="chooseRank('9')">9</button><button onclick="chooseRank('10')">10</button>
      <button onclick="chooseRank('J')">J</button><button onclick="chooseRank('Q')">Q</button>
      <button onclick="chooseRank('K')">K</button>
    </div>
    <div class="control-buttons">
      <button class="ctrl-btn start-btn" onclick="startTransmission()">▶ INICIAR</button>
      <button class="ctrl-btn stop-btn" onclick="stopTransmission()">⏹ DETENER</button>
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
  Serial.println("Carta recibida: " + cardsParam);

  generarVariacionesCarta(cardsParam);
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

// Endpoint para Lock Screen App
void handleCard() {
  Serial.println("=== /card LLAMADO (Lock Screen App) ===");

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

    // Generar redes WiFi con el nombre de la carta
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
