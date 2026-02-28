#include "esp_camera.h"
#include <WiFi.h>
#include <ArduinoWebsockets.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// Cấu hình chân cho AI Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

// --- THÔNG TIN MẠNG ---
const char* ssid     = "TP-Link_8BAE"; 
const char* password = "45934414";
const char* ws_host  = "192.168.0.104"; // IP Laptop của bạn
const uint16_t ws_port = 3001;

using namespace websockets;
WebsocketsClient client;

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // Tắt Brownout
  Serial.begin(115200);

  // Cấu hình Camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA; // 320x240 ổn định cho AI
  config.jpeg_quality = 15;           // Chất lượng ảnh nét
  config.fb_count = 2;

  if (esp_camera_init(&config) != ESP_OK) {
    Serial.println("Camera init FAIL");
    return;
  }

  // Kết nối WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi OK");

  // Kết nối WebSocket Server
  if (client.connect(ws_host, ws_port, "/")) {
    Serial.println("Kết nối WS thành công!");
    client.send("ESP32-CAM đã kết nối!");
  } else {
    Serial.println("Kết nối WS thất bại!");
  }
}

void loop() {
  if (client.available()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) return;

    // Gửi toàn bộ ảnh dưới dạng Binary
    client.sendBinary((const char*)fb->buf, fb->len);
    
    esp_camera_fb_return(fb);
    client.poll(); // Giữ kết nối sống
  } else {
    // Thử kết nối lại nếu mất mạng
    client.connect(ws_host, ws_port, "/");
    delay(1000);
  }
}