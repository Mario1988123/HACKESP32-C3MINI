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

// WiFi AP Configuration
const char* ssid = "ESP32-LockScreen";
const char* password = "12345678";

// Web Server
WebServer server(80);

// Card symbols using simple characters
const char* HEART = "♥";
const char* SPADE = "♠";
const char* CLUB = "♣";
const char* DIAMOND = "♦";

// Buffer to store received cards
String lastCards[5];
int cardCount = 0;

void setup() {
  Serial.begin(115200);

  // Initialize display
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println("Iniciando...");
  display.display();

  // Configure WiFi Access Point
  Serial.println("Configurando Access Point...");
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  // Display WiFi info
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("WiFi AP Activo");
  display.println();
  display.print("SSID: ");
  display.println(ssid);
  display.print("IP: ");
  display.println(IP);
  display.println();
  display.println("Esperando cartas...");
  display.display();

  // Configure server endpoints
  server.on("/card", HTTP_POST, handleCardPost);
  server.on("/", HTTP_GET, handleRoot);

  server.begin();
  Serial.println("Servidor HTTP iniciado");
}

void loop() {
  server.handleClient();
}

void handleRoot() {
  String html = "<html><body>";
  html += "<h1>ESP32 Lock Screen Card Receiver</h1>";
  html += "<p>AP IP: " + WiFi.softAPIP().toString() + "</p>";
  html += "<p>Esperando cartas desde la app Android...</p>";
  html += "<h2>Últimas cartas recibidas:</h2>";
  html += "<ul>";
  for (int i = 0; i < cardCount && i < 5; i++) {
    html += "<li>" + lastCards[i] + "</li>";
  }
  html += "</ul>";
  html += "</body></html>";

  server.send(200, "text/html", html);
}

void handleCardPost() {
  if (server.hasArg("plain")) {
    String body = server.arg("plain");
    Serial.println("Recibido: " + body);

    // Parse JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);

    if (error) {
      Serial.print("Error parsing JSON: ");
      Serial.println(error.c_str());
      server.send(400, "text/plain", "Invalid JSON");
      return;
    }

    const char* card = doc["card"];
    if (card) {
      String cardStr = String(card);
      Serial.println("Carta recibida: " + cardStr);

      // Add to buffer
      if (cardCount < 5) {
        lastCards[cardCount] = cardStr;
        cardCount++;
      } else {
        // Shift array and add new card
        for (int i = 0; i < 4; i++) {
          lastCards[i] = lastCards[i + 1];
        }
        lastCards[4] = cardStr;
      }

      // Display card on OLED
      displayCard(cardStr);

      server.send(200, "application/json", "{\"status\":\"ok\"}");
    } else {
      server.send(400, "text/plain", "Missing 'card' field");
    }
  } else {
    server.send(400, "text/plain", "No body received");
  }
}

void displayCard(String card) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Carta Recibida:");
  display.println();

  // Parse card name to extract suit and value
  // Format: "Value of Suit"
  display.setTextSize(2);

  // Extract suit and show symbol
  String symbol = "";
  if (card.indexOf("Hearts") > 0) {
    symbol = HEART;
  } else if (card.indexOf("Spades") > 0) {
    symbol = SPADE;
  } else if (card.indexOf("Clubs") > 0) {
    symbol = CLUB;
  } else if (card.indexOf("Diamonds") > 0) {
    symbol = DIAMOND;
  }

  // Extract value (before " of ")
  int ofIndex = card.indexOf(" of ");
  String value = "";
  if (ofIndex > 0) {
    value = card.substring(0, ofIndex);
  }

  // Center the card display
  display.setCursor(10, 20);
  display.print(symbol);
  display.print(" ");
  display.println(value);

  // Show full card name at bottom in small text
  display.setTextSize(1);
  display.setCursor(0, 50);
  if (card.length() > 21) {
    display.print(card.substring(0, 21));
  } else {
    display.print(card);
  }

  display.display();

  Serial.println("Carta mostrada en display: " + card);
}
