import cv2
import json
import time
import numpy as np
import mediapipe as mp
import paho.mqtt.client as mqtt
from mediapipe.tasks import python
from mediapipe.tasks.python import vision


# Đường dẫn đến model hand_landmarker  
MODEL_PATH = "hand_landmarker.task"

# Khởi tạo các lớp từ Mediapipe
BaseOptions = python.BaseOptions
HandLandmarker = vision.HandLandmarker
HandLandmarkerOptions = vision.HandLandmarkerOptions
VisionRunningMode = vision.RunningMode

# Cấu hình model
options = HandLandmarkerOptions(
    base_options=BaseOptions(model_asset_path=MODEL_PATH),
    running_mode=VisionRunningMode.VIDEO,
    num_hands=2,
    min_hand_detection_confidence=0.6,
    min_hand_presence_confidence=0.6,
    min_tracking_confidence=0.6
)

# Cấu hình MQTT
BROKER = "6419f78d6e5e4affbebe010720192414.s1.eu.hivemq.cloud"
Web_Sockets_PORT = 8884
PORT = 8883

# Thiết lập MQTT client với WebSockets và TLS
client = mqtt.Client(transport="websockets")
client.ws_set_options(path="/mqtt")
client.tls_set()  # Sử dụng TLS cho kết nối an toàn
client.username_pw_set("NCKH2026", "Nckh-2026")
client.connect(BROKER, Web_Sockets_PORT)
client.loop_start()

# Hàm điều khiển LED qua MQTT
def led_on():
    pay_load = json.dumps({"led1": 1}) 
    client.publish("data", pay_load, qos=1)  # QoS 1 để đảm bảo tin nhắn được gửi đi
def led_off():
    pay_load = json.dumps({"led1": 0})
    client.publish("data", pay_load, qos=1)  # QoS 1 để đảm bảo tin nhắn được gửi đi

# Hàm kiểm tra nếu bàn tay mở (dựa trên vị trí của các điểm đặc trưng)
def is_hand_open(landmarks):
    finger_tips = [8,12,16,20]
    finger_pips = [6,10,14,18]
    open_count = 0

# Kiểm tra nếu ngón tay nào đó đang mở (tip cao hơn pip)
    for tip, pip in zip(finger_tips, finger_pips):
        if landmarks[tip].y < landmarks[pip].y:
            open_count += 1

    return open_count >= 3

# Mở webcam 
# ESP32_CAM = "http://192.168.0.105:81/stream"
# cap = cv2.VideoCapture(ESP32_CAM)
cap = cv2.VideoCapture(0)

status = 0
last_status = -1


frame_count = 0
process_count = 0
fps_timer = time.time()
frame_count_display = 0
process_count_display = 0

# Sử dụng model để phát hiện bàn tay và điều khiển LED
with HandLandmarker.create_from_options(options) as landmarker:

    while cap.isOpened():
        ret, frame = cap.read()
        frame_count += 1
        if not ret:
            break 

        frame = cv2.flip(frame, 1)
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
        mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)

        result = landmarker.detect_for_video(mp_image, int(cap.get(cv2.CAP_PROP_POS_MSEC)))
        process_count += 1

        open_hands = 0
        fist_hands = 0

        if result.hand_landmarks:
            for hand_landmarks in result.hand_landmarks:
                for lm in hand_landmarks:
                    x = int(lm.x * frame.shape[1])
                    y = int(lm.y * frame.shape[0])
                    cv2.circle(frame, (x,y), 4, (0,255,0), -1)

                if is_hand_open(hand_landmarks):
                    open_hands += 1
                else:
                    fist_hands += 1

        # Logic điều khiển
        if fist_hands >= 1:
            status = 0
        elif open_hands >= 1:
            status = 1

        text = "ON (1)" if status else "OFF (0)"
        color = (0,255,0) if status else (0,0,255)

        cv2.putText(frame, f"STATUS: {text}", (20,50),
                    cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3)
        if(time.time() - fps_timer) >= 1.0:
            frame_count_display = frame_count
            process_count_display = process_count
            fps_timer = time.time()
            frame_count = 0
            process_count = 0
        cv2.putText(frame, f"FPS: {process_count_display} / {frame_count_display}", (20,100),
        cv2.FONT_HERSHEY_SIMPLEX, 1.0, (255,255,0), 2)
        cv2.imshow("Hand Control", frame)
        print(status)
        if status != last_status:
            if status == 1:
                led_on()   
            else:
                led_off()   
            last_status = status

        if cv2.waitKey(1) & 0xFF == 27:
            break

cap.release()
cv2.destroyAllWindows()