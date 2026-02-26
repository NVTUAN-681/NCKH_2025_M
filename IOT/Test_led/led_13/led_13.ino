#define LED_PIN_13 13
// #define LED_PIN_4 4

void setup() 
{
// pinMode(LED_PIN_4, OUTPUT);   
pinMode(LED_PIN_13, OUTPUT); 
Serial.begin(115200);
}  

void loop() {   
  // digitalWrite(LED_PIN_4, LOW);  // Bật LED 
  digitalWrite(LED_PIN_13, LOW);
  Serial.print("off\n");
  delay(500);
  digitalWrite(LED_PIN_13, HIGH);
  Serial.print("on\n");
  delay(500);
}
