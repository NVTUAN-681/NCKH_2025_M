#include <ArduinoJson.h>

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <MQTTPubSubClient.h>

WebSocketsClient client;
MQTTPubSubClient mqtt;

const char* ssid = "TP-Link_8BAE";
const char* pass = "45934414";

#define ledPin 14
// #define ledPin2 13

void setup() {
  Serial.begin(115200);

pinMode(ledPin,OUTPUT);
// pinMode(ledPin2,OUTPUT);

  Serial.print("connecting to wifi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }

  Serial.println(" connected! ");
  Serial.println("connecting to host...");

  mqtt.begin(client);
  client.disconnect();

  const char* mqtt_server = "6419f78d6e5e4affbebe010720192414.s1.eu.hivemq.cloud";
  client.beginSSL(mqtt_server, 8884, "/mqtt");
  client.setReconnectInterval(2000);

  Serial.print("connecting to mqtt broker...");
  while(!mqtt.connect("ESP32", "NCKH2026", "Nckh-2026")){
    Serial.print(".");
    delay(500);
  }
  Serial.print(" connected");

  mqtt.subscribe("data", [](const String& payload, const size_t size){
    Serial.print("Data: "); Serial.println(payload);
    //data nhan duoc {"led1":0,"led2":1}

    StaticJsonDocument<200> doc;
    //phan tich cu phap json
    DeserializationError error = deserializeJson(doc, payload);
    //kiem tra loi
    if(error){
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    JsonObject obj = doc.as<JsonObject>();
    if(obj.containsKey("led1")){
      bool ledState = doc["led1"];
      digitalWrite(ledPin,ledState);
      int state = digitalRead(ledPin);
      mqtt.publish("led1", String(state));
    }

    // if(obj.containsKey("led2")){
    //   bool ledState2 = doc["led2"];
    //   digitalWrite(ledPin2,ledState2);
    //   int state = digitalRead(ledPin2);
    //   mqtt.publish("led2", String(state));
    // }
  });
}

void loop(){
  mqtt.update(); // phai co dong nay
  static uint32_t prev_ms = millis();
  if (millis()> prev_ms + 10000){
    prev_ms = millis();
    String data = String(millis()/1000);
    mqtt.publish("uptime", data);
  }
}