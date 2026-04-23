#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <MQTTPubSubClient.h>
#include <ESP32Servo.h> 
#include <NTPClient.h>
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 7 * 3600); // Múi giờ +7 (Việt Nam)

Servo myServo;
WebSocketsClient client;
MQTTPubSubClient mqtt;

const char* ssid = "Want";
const char* pass = "87654321";

#define Living_light 13
#define Kitchen_light 14
#define Door 18

// Biến lưu trữ trạng thái để đồng bộ
bool status_living = false;
bool status_kitchen = false;
int status_door = 0; // 0: Đóng, 1: Mở

struct DeviceSchedule {
    bool timerActive = false;
    long startTimeSec = -1; // Giờ bắt đầu tính bằng giây trong ngày
    long endTimeSec = -1;   // Giờ kết thúc tính bằng giây trong ngày
    long remainStart = 0;   // Giây còn lại đến khi BẬT
    long remainEnd = 0;     // Giây còn lại đến khi TẮT
};

DeviceSchedule schedLiving, schedKitchen, schedDoor;

long timeToSeconds(String timeStr) {
    if (timeStr == "") return -1;
    int firstColon = timeStr.indexOf(':');
    int hour = timeStr.substring(0, firstColon).toInt();
    int minute = timeStr.substring(firstColon + 1).toInt();
    return (hour * 3600L) + (minute * 60L);
}

void updateAndPublishSchedules() {
    timeClient.update();
    long now = (timeClient.getHours() * 3600L) + (timeClient.getMinutes() * 60L) + timeClient.getSeconds();
    
    // Logic tính toán cho từng thiết bị (Ví dụ cho Living Light)
    auto updateDevice = [&](DeviceSchedule &s, int pin, bool isServo = false) {
        if (s.timerActive) {
            // Tính remainStart
            if (now < s.startTimeSec) s.remainStart = s.startTimeSec - now;
            else s.remainStart = 0;

            // Tính remainEnd
            if (now < s.endTimeSec) s.remainEnd = s.endTimeSec - now;
            else s.remainEnd = 0;

            // Thực thi lệnh khi đến giờ
            if (s.remainStart == 0 && now < s.endTimeSec) {
                 if(!isServo) digitalWrite(pin, HIGH); else myServo.write(90);
            }
            if (s.remainEnd == 0) {
                 if(!isServo) digitalWrite(pin, LOW); else myServo.write(0);
                 s.timerActive = false; // Kết thúc lịch trình
            }
        }
    };

    updateDevice(schedLiving, Living_light);
    updateDevice(schedKitchen, Kitchen_light);
    updateDevice(schedDoor, Door, true);

    // Gửi JSON gộp về Web
    StaticJsonDocument<512> root;
    JsonObject dev = root.createNestedObject("devices");
    
    auto addData = [&](const char* name, DeviceSchedule &s, bool st) {
        JsonObject obj = dev.createNestedObject(name);
        obj["val"] = st;
        obj["timer_active"] = s.timerActive;
        obj["rem_s"] = s.remainStart;
        obj["rem_e"] = s.remainEnd;
    };

    addData("Living_light", schedLiving, digitalRead(Living_light));
    addData("Kitchen_light", schedKitchen, digitalRead(Kitchen_light));
    addData("Door", schedDoor, (status_door == 1));
    
    root["esp_time"] = timeClient.getFormattedTime();

    char buffer[512];
    serializeJson(root, buffer);
    mqtt.publish("home/state", buffer);
}

void publishFullStatus() {
  timeClient.update(); // Cập nhật giờ mới nhất từ Internet
  String formattedTime = timeClient.getFormattedTime();
  
  StaticJsonDocument<256> statusDoc;
  statusDoc["Living_light"] = status_living;
  statusDoc["Kitchen_light"] = status_kitchen;
  statusDoc["Door"] = status_door;
  statusDoc["esp_time"] = formattedTime; // Gửi giờ thực của ESP về Web
  statusDoc["status"] = "synchronized";

  char buffer[256];
  serializeJson(statusDoc, buffer);
  
  // Debug: Ghi lại log gửi đi
  Serial.print("[DEBUG] Gửi trạng thái về Web lúc: ");
  Serial.println(formattedTime);
  
  mqtt.publish("home/state", buffer);
}

void setup() {
  Serial.begin(115200);

  pinMode(Living_light, OUTPUT);
  pinMode(Kitchen_light, OUTPUT);

  myServo.attach(Door, 500, 2400);
  myServo.write(0); 

  Serial.print("connecting to wifi...");
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(1000);
  }

  Serial.println(" connected! ");
  
  mqtt.begin(client);
  const char* mqtt_server = "6419f78d6e5e4affbebe010720192414.s1.eu.hivemq.cloud";
  client.beginSSL(mqtt_server, 8884, "/mqtt");
  client.setReconnectInterval(2000);

  Serial.print("connecting to mqtt broker...");
  while(!mqtt.connect("ESP32", "NCKH2026", "Nckh-2026")){
    Serial.print(".");
    delay(500);
  }
  Serial.println(" connected");

// Đăng ký nhận lệnh lẻ từ Web qua topic mới
// Đăng ký nhận lệnh từ Web (Topic nâng cấp)
  mqtt.subscribe("home/commands", [](const String& payload, const size_t size) {
    timeClient.update();
    Serial.println("\n------------------------------------");
    Serial.print("[DEBUG] Lệnh nhận lúc: "); Serial.println(timeClient.getFormattedTime());
    Serial.print("[PAYLOAD]: "); Serial.println(payload);

    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if (error) return;

    JsonObject obj = doc.as<JsonObject>();
    String device = obj["device"] | "";

    // 1. XỬ LÝ LỆNH ĐIỀU KHIỂN TỨC THỜI (Lệnh lẻ)
    if (obj.containsKey("Living_light")) {
      status_living = obj["Living_light"];
      digitalWrite(Living_light, status_living);
    }
    if (obj.containsKey("Kitchen_light")) {
      status_kitchen = obj["Kitchen_light"];
      digitalWrite(Kitchen_light, status_kitchen);
    }
    if (obj.containsKey("Door")) {
      status_door = obj["Door"];
      if (status_door == 1) myServo.write(90); else myServo.write(0);
    }

    // 2. XỬ LÝ THIẾT LẬP LỊCH TRÌNH (Independent Scheduler)
    if (obj.containsKey("start_time") && obj.containsKey("end_time")) {
      DeviceSchedule *target = nullptr;
      if (device == "Living_light") target = &schedLiving;
      else if (device == "Kitchen_light") target = &schedKitchen;
      else if (device == "Door") target = &schedDoor;

      if (target != nullptr) {
        target->startTimeSec = timeToSeconds(obj["start_time"]);
        target->endTimeSec = timeToSeconds(obj["end_time"]);
        target->timerActive = true;
        Serial.printf("[INFO] Đã đặt lịch cho %s: %s -> %s\n", 
                      device.c_str(), obj["start_time"].as<char*>(), obj["end_time"].as<char*>());
      }
    }

    // 3. XỬ LÝ HỦY LỊCH TRÌNH
    if (obj["action"] == "clear_schedule") {
      if (device == "Living_light") schedLiving.timerActive = false;
      else if (device == "Kitchen_light") schedKitchen.timerActive = false;
      else if (device == "Door") schedDoor.timerActive = false;
      Serial.printf("[INFO] Đã hủy lịch trình của: %s\n", device.c_str());
    }

    // Gửi phản hồi gộp ngay lập tức sau khi nhận lệnh
    updateAndPublishSchedules();
    Serial.println("------------------------------------");
  });
}


void loop() {
  // Duy trì kết nối MQTT và WebSocket
  client.loop();
  mqtt.update();

  // Kiểm tra kết nối WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[ERROR] Mất WiFi, đang kết nối lại...");
    WiFi.begin(ssid, pass);
    delay(5000);
    return;
  }

  // CƠ CHẾ ĐỒNG BỘ MỖI 1 GIÂY
  static uint32_t lastTick = 0;
  if (millis() - lastTick >= 1000) {
    lastTick = millis();
    
    // Cập nhật giờ từ NTP
    timeClient.update();
    
    // Tính toán lịch trình, thực thi lệnh và gửi dữ liệu gộp về Web
    updateAndPublishSchedules();
    
    // Gửi Heartbeat định kỳ (10s/lần) lên topic giám sát
    static int heartbeatCount = 0;
    if (++heartbeatCount >= 10) {
      mqtt.publish("home/heartbeat", String(millis() / 1000));
      heartbeatCount = 0;
    }
  }
}