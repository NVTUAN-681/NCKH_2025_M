#include <ESP32Servo.h>

Servo myServo;
const int servoPin = 18; // Chân bạn đã chọn

void setup() {
  Serial.begin(115200);

  // Vẫn giữ sự đơn giản, nhưng thêm dải xung để MG90S chạy mượt nhất
  // 500: xung ứng với 0 độ, 2400: xung ứng với 180 độ
  myServo.attach(servoPin, 500, 2400); 

  Serial.println("Servo chuẩn bị sẵn sàng cho Smart Home!");
}

void loop() {
  myServo.write(0);
  delay(2000);
  myServo.write(180);
  delay(2000);
}