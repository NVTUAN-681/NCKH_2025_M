import threading
import cv2
import json
import time
import socket
import queue
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

# --- CẤU HÌNH UDP ---
UDP_IP = "0.0.0.0" 
UDP_PORT = 12345
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
frame_queue = queue.Queue(maxsize=1)

# --- CẤU HÌNH MQTT ---
BROKER = "6419f78d6e5e4affbebe010720192414.s1.eu.hivemq.cloud"
Web_Sockets_PORT = 8884
PORT = 8883

# Thiết lập MQTT client với WebSockets và TLS
client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, transport="websockets")
client.ws_set_options(path="/mqtt")
client.tls_set()  
client.username_pw_set("NCKH2026", "Nckh-2026")
client.connect(BROKER, Web_Sockets_PORT)
client.loop_start()

# Luồng nhận video UDP
def receive_udp_stream():
    global frame_count
    while True:
        try:
            data, addr = sock.recvfrom(65535)
            np_data = np.frombuffer(data, dtype=np.uint8)
            frame = cv2.imdecode(np_data, cv2.IMREAD_COLOR)
            if frame is not None:
                # Tăng đếm khung hình đầu vào
                if not frame_queue.full():
                    frame_queue.put(frame)
        except Exception as e:
            print(f"Lỗi nhận dữ liệu: {e}")

threading.Thread(target=receive_udp_stream, daemon=True).start()    

# cap = cv2.VideoCapture(0)  # Sử dụng webcam nếu không có stream UDP

def led_on():
    pay_load = json.dumps({"led1": 1}) 
    client.publish("data", pay_load, qos=1)
def led_off():
    pay_load = json.dumps({"led1": 0})
    client.publish("data", pay_load, qos=1)

def is_hand_open(landmarks):
    finger_tips = [8,12,16,20]; finger_pips = [6,10,14,18]
    open_count = 0
    for tip, pip in zip(finger_tips, finger_pips):
        if landmarks[tip].y < landmarks[pip].y:
            open_count += 1
    return open_count >= 3

status = 0
last_status = -1
AI_FPS = 10  # Tăng lên 10 để nhạy hơn
interval = 1.0 / AI_FPS
last_process_time = 0
frame_count = 0
process_count = 0
frame_count_display = 0
process_count_display = 0
fps_timer = time.time()

with HandLandmarker.create_from_options(options) as landmarker: 
    while True:
        print("Bắt đầu xử lý video...")
        if not frame_queue.empty():
            print("Đang xử lý khung hình...")
            frame = frame_queue.get()
            frame_count += 1 # Đếm khung hình nhận được
            frame = cv2.flip(frame, 1) # Chỉ flip 1 lần
    # while cap.isOpened():
            # ret, frame = cap.read()
            # frame_count += 1
            # if not ret:            continue        
            current_time = time.time()
            if current_time - last_process_time >= interval:
                last_process_time = current_time
                process_count += 1
                
                rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
                mp_image = mp.Image(image_format=mp.ImageFormat.SRGB, data=rgb)
                result = landmarker.detect_for_video(mp_image, int(current_time * 1000))
                
                open_hands = 0
                fist_hands = 0
                if result.hand_landmarks:
                    for hand_landmarks in result.hand_landmarks:
                        if is_hand_open(hand_landmarks):
                            open_hands += 1
                        else:
                            fist_hands += 1

                if fist_hands >= 1:
                    status = 0
                elif open_hands >= 1:
                    status = 1

                if status != last_status:
                    if status == 1: led_on()
                    else: led_off()
                    last_status = status
                    print(f"MQTT Sent Status: {status}")

            # Cập nhật hiển thị FPS mỗi giây
            if (current_time - fps_timer) >= 1.0:
                frame_count_display = frame_count
                process_count_display = process_count
                frame_count = 0
                process_count = 0
                fps_timer = current_time

            # Vẽ thông tin lên frame
            text = "ON" if status else "OFF"
            color = (0,255,0) if status else (0,0,255)
            cv2.putText(frame, f"STATUS: {text}", (20,50), cv2.FONT_HERSHEY_SIMPLEX, 1.2, color, 3)
            cv2.putText(frame, f"FPS: {process_count_display} (AI) / {frame_count_display} (Video)", (20,100),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,0), 2)

            cv2.imshow("Smart Control System", frame)

        if cv2.waitKey(1) & 0xFF == 27:
            break

cv2.destroyAllWindows()