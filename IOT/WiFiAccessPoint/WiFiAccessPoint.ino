#include <WiFi.h>
#include <WebServer.h>
#include <WiFiAP.h>

const char* ssid = "ESP32_LED";
const char* password = "12345678";

WebServer server(80);
const int ledPin = 2;

/* ================== HTML Ở ĐÂY ================== */
const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 LED Control</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body style="text-align:center; font-family: Arial;">
  <h2>ESP32 Web Control</h2>
  <button onclick="fetch('/on')">LED ON</button>
  <button onclick="fetch('/off')">LED OFF</button>
</body>
</html>
)rawliteral";
/* ================================================= */

void handleRoot() {
  server.send(200, "text/html", webpage);
}

void handleLEDOn() {
  digitalWrite(ledPin, HIGH);
  server.send(200, "text/plain", "LED ON");
}

void handleLEDOff() {
  digitalWrite(ledPin, LOW);
  server.send(200, "text/plain", "LED OFF");
}

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  WiFi.softAP(ssid, password);
  Serial.begin(115200);
  Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/on", handleLEDOn);
  server.on("/off", handleLEDOff);

  server.begin();
}

void loop() {
  server.handleClient();
}
