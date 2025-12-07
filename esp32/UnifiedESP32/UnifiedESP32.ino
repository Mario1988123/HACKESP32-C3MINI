/*
 * ESP32-C3 Unified System
 * Supports both WiFi Calculator and Lock Screen Card modes
 *
 * Mode switching:
 * - Press BOOT button (GPIO9) to toggle between modes
 * - Automatically detects data type (calculator operation or card)
 *
 * Modes:
 * 1. CALCULATOR MODE: Receives math operations (e.g., "5+3=8")
 * 2. LOCKSCREEN MODE: Receives playing cards (e.g., "Ace of Hearts")
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Mode button (BOOT button on most ESP32-C3 boards)
#define MODE_BUTTON 9

// WiFi AP Configuration
const char* ssid = "ESP32-Hidden";
const char* password = "12345678";

// Web Server
WebServer server(80);

// Operating modes
enum Mode {
  MODE_CALCULATOR,
  MODE_LOCKSCREEN
};

Mode currentMode = MODE_LOCKSCREEN; // Default mode
unsigned long lastModeSwitch = 0;
const unsigned long MODE_SWITCH_DEBOUNCE = 500;

// Card symbols
const char* HEART = "♥";
const char* SPADE = "♠";
const char* CLUB = "♣";
const char* DIAMOND = "♦";

// Data buffers
String lastOperations[5];
String lastCards[5];
int operationCount = 0;
int cardCount = 0;

void setup() {
  Serial.begin(115200);

  // Setup mode button
  pinMode(MODE_BUTTON, INPUT_PULLUP);

  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  displayStartup();

  // Configure WiFi Access Point
  Serial.println("Configurando Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  displayWiFiInfo(IP);

  // Configure server endpoints
  server.on("/", HTTP_GET, handleRoot);
  server.on("/calc", HTTP_POST, handleCalculator);
  server.on("/card", HTTP_POST, handleCard);
  server.on("/data", HTTP_POST, handleData); // Universal endpoint
  server.on("/mode", HTTP_GET, handleModeGet);
  server.on("/mode", HTTP_POST, handleModeSet);

  server.begin();
  Serial.println("Servidor HTTP iniciado");

  displayModeScreen();
}

void loop() {
  server.handleClient();
  checkModeButton();
}

void checkModeButton() {
  if (digitalRead(MODE_BUTTON) == LOW) {
    unsigned long now = millis();
    if (now - lastModeSwitch > MODE_SWITCH_DEBOUNCE) {
      toggleMode();
      lastModeSwitch = now;
      while(digitalRead(MODE_BUTTON) == LOW) {
        delay(10);
      }
    }
  }
}

void toggleMode() {
  if (currentMode == MODE_CALCULATOR) {
    currentMode = MODE_LOCKSCREEN;
    Serial.println("Modo: LOCK SCREEN");
  } else {
    currentMode = MODE_CALCULATOR;
    Serial.println("Modo: CALCULATOR");
  }
  displayModeScreen();
}

void displayStartup() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("ESP32 Unified");
  display.println("System");
  display.println();
  display.println("Iniciando...");
  display.display();
  delay(1000);
}

void displayWiFiInfo(IPAddress IP) {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi AP Activo");
  display.println();
  display.print("SSID: ");
  display.println(ssid);
  display.print("Pass: ");
  display.println(password);
  display.print("IP: ");
  display.println(IP);
  display.display();
  delay(2000);
}

void displayModeScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);

  if (currentMode == MODE_CALCULATOR) {
    display.println("CALC");
    display.setTextSize(1);
    display.println();
    display.println("Modo:");
    display.println("Calculadora");
  } else {
    display.println("LOCK");
    display.setTextSize(1);
    display.println();
    display.println("Modo:");
    display.println("Pantalla Bloq.");
  }

  display.println();
  display.println("Esperando...");
  display.display();
}

void handleRoot() {
  String html = "<html><head><meta charset='utf-8'></head><body>";
  html += "<h1>ESP32 Unified System</h1>";
  html += "<p>AP IP: " + WiFi.softAPIP().toString() + "</p>";
  html += "<h2>Modo Actual: ";
  html += (currentMode == MODE_CALCULATOR) ? "CALCULADORA" : "PANTALLA DE BLOQUEO";
  html += "</h2>";

  html += "<h3>Cambiar Modo:</h3>";
  html += "<form action='/mode' method='POST'>";
  html += "<button name='mode' value='calculator'>Modo Calculadora</button> ";
  html += "<button name='mode' value='lockscreen'>Modo Pantalla Bloqueo</button>";
  html += "</form>";

  if (currentMode == MODE_CALCULATOR && operationCount > 0) {
    html += "<h3>Últimas Operaciones:</h3><ul>";
    for (int i = 0; i < operationCount && i < 5; i++) {
      html += "<li>" + lastOperations[i] + "</li>";
    }
    html += "</ul>";
  }

  if (currentMode == MODE_LOCKSCREEN && cardCount > 0) {
    html += "<h3>Últimas Cartas:</h3><ul>";
    for (int i = 0; i < cardCount && i < 5; i++) {
      html += "<li>" + lastCards[i] + "</li>";
    }
    html += "</ul>";
  }

  html += "</body></html>";
  server.send(200, "text/html; charset=utf-8", html);
}

void handleModeGet() {
  String mode = (currentMode == MODE_CALCULATOR) ? "calculator" : "lockscreen";
  String json = "{\"mode\":\"" + mode + "\"}";
  server.send(200, "application/json", json);
}

void handleModeSet() {
  if (server.hasArg("mode")) {
    String mode = server.arg("mode");
    if (mode == "calculator") {
      currentMode = MODE_CALCULATOR;
      displayModeScreen();
      server.send(200, "application/json", "{\"status\":\"ok\",\"mode\":\"calculator\"}");
    } else if (mode == "lockscreen") {
      currentMode = MODE_LOCKSCREEN;
      displayModeScreen();
      server.send(200, "application/json", "{\"status\":\"ok\",\"mode\":\"lockscreen\"}");
    } else {
      server.send(400, "text/plain", "Invalid mode");
    }
  } else {
    server.send(400, "text/plain", "Missing mode parameter");
  }
}

void handleData() {
  // Universal endpoint that auto-detects data type
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Recibido en /data: " + body);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    // Auto-detect data type
    if (doc.containsKey("operation") || doc.containsKey("result")) {
      // Calculator data
      processCalculatorData(doc);
    } else if (doc.containsKey("card")) {
      // Card data
      processCardData(doc);
    } else {
      server.send(400, "text/plain", "Unknown data type");
      return;
    }

    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "No body received");
  }
}

void handleCalculator() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Recibido en /calc: " + body);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    processCalculatorData(doc);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "No body received");
  }
}

void handleCard() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Recibido en /card: " + body);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    processCardData(doc);
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "text/plain", "No body received");
  }
}

void processCalculatorData(JsonDocument& doc) {
  String operation = "";

  if (doc.containsKey("operation")) {
    operation = doc["operation"].as<String>();
  } else if (doc.containsKey("num1") && doc.containsKey("num2") && doc.containsKey("operator")) {
    // Build operation string from components
    operation = String(doc["num1"].as<String>()) + " " +
                doc["operator"].as<String>() + " " +
                doc["num2"].as<String>();

    if (doc.containsKey("result")) {
      operation += " = " + doc["result"].as<String>();
    }
  }

  if (operation.length() > 0) {
    Serial.println("Operación: " + operation);

    // Add to buffer
    if (operationCount < 5) {
      lastOperations[operationCount] = operation;
      operationCount++;
    } else {
      for (int i = 0; i < 4; i++) {
        lastOperations[i] = lastOperations[i + 1];
      }
      lastOperations[4] = operation;
    }

    displayCalculator(operation);
  }
}

void processCardData(JsonDocument& doc) {
  const char* card = doc["card"];

  if (card) {
    String cardStr = String(card);
    Serial.println("Carta: " + cardStr);

    // Add to buffer
    if (cardCount < 5) {
      lastCards[cardCount] = cardStr;
      cardCount++;
    } else {
      for (int i = 0; i < 4; i++) {
        lastCards[i] = lastCards[i + 1];
      }
      lastCards[4] = cardStr;
    }

    displayCard(cardStr);
  }
}

void displayCalculator(String operation) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CALCULADORA");
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  display.setCursor(0, 16);
  display.setTextSize(2);

  // Display operation, split if too long
  if (operation.length() > 10) {
    int eqPos = operation.indexOf('=');
    if (eqPos > 0) {
      display.println(operation.substring(0, eqPos));
      display.print("= ");
      display.println(operation.substring(eqPos + 1));
    } else {
      display.println(operation.substring(0, 10));
      display.println(operation.substring(10));
    }
  } else {
    display.println(operation);
  }

  display.display();
  Serial.println("Operación mostrada: " + operation);
}

void displayCard(String card) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("CARTA RECIBIDA");
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);

  // Parse card name
  String symbol = "";
  if (card.indexOf("Hearts") >= 0 || card.indexOf("Corazones") >= 0) {
    symbol = HEART;
  } else if (card.indexOf("Spades") >= 0 || card.indexOf("Picas") >= 0) {
    symbol = SPADE;
  } else if (card.indexOf("Clubs") >= 0 || card.indexOf("Treboles") >= 0) {
    symbol = CLUB;
  } else if (card.indexOf("Diamonds") >= 0 || card.indexOf("Diamantes") >= 0) {
    symbol = DIAMOND;
  }

  // Extract value
  int ofIndex = card.indexOf(" of ");
  if (ofIndex < 0) ofIndex = card.indexOf(" de ");
  String value = "";
  if (ofIndex > 0) {
    value = card.substring(0, ofIndex);
  } else {
    value = card;
  }

  // Display card
  display.setTextSize(2);
  display.setCursor(10, 20);
  display.print(symbol);
  display.print(" ");

  // Adjust font size if value is too long
  if (value.length() > 5) {
    display.setTextSize(1);
    display.setCursor(30, 28);
  }
  display.println(value);

  // Show full card name at bottom
  display.setTextSize(1);
  display.setCursor(0, 50);
  if (card.length() > 21) {
    display.print(card.substring(0, 21));
  } else {
    display.print(card);
  }

  display.display();
  Serial.println("Carta mostrada: " + card);
}
