// 1. Cấu hình thông số HiveMQ (Khớp với Python và ESP32)
const BROKER = "6419f78d6e5e4affbebe010720192414.s1.eu.hivemq.cloud";
const PORT = 8884;
const CLIENT_ID = "web_client_" + Math.random().toString(16).substr(2, 8);
const USER = "NCKH2026";
const PASS = "Nckh-2026";

const TOPIC_SET = "data";    // Gửi lệnh đi
const TOPIC_STATUS = "data"; // Nhận trạng thái về

// 2. Khởi tạo MQTT Client
const client = new Paho.MQTT.Client(BROKER, PORT, CLIENT_ID);

// 3. Các hàm xử lý sự kiện
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

const options = {
    useSSL: true,
    userName: USER,
    password: PASS,
    onSuccess: onConnect,
    onFailure: onFail
};

client.connect(options);

function onConnect() {
    console.log("Đã kết nối HiveMQ!");
    document.getElementById("connection-status").innerText = "Đã kết nối Cloud";
    document.getElementById("connection-status").style.color = "green";
    // Đăng ký nhận trạng thái từ ESP32
    client.subscribe(TOPIC_STATUS);
}

function onFail(e) {
    console.log("Kết nối thất bại: " + e.errorMessage);
    document.getElementById("connection-status").innerText = "Lỗi kết nối!";
}

function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
        document.getElementById("connection-status").innerText = "Mất kết nối!";
    }
}

// 4. Nhận dữ liệu và cập nhật giao diện
function onMessageArrived(message) {
    console.log("Dữ liệu về: " + message.payloadString);
    try {
        let data = JSON.parse(message.payloadString);
        let statusLight = document.getElementById("status-light");
        let textStatus = document.getElementById("text-status");

        if (data.led1 === 1) {
            statusLight.style.backgroundColor = "#ffeb3b"; // Màu vàng sáng
            statusLight.style.boxShadow = "0 0 15px #ffeb3b";
            textStatus.innerText = "Trạng thái: ĐANG BẬT";
        } else {
            statusLight.style.backgroundColor = "gray";
            statusLight.style.boxShadow = "none";
            textStatus.innerText = "Trạng thái: ĐANG TẮT";
        }
    } catch (e) {
        console.log("Lỗi giải mã JSON");
    }
}

// 5. Gửi lệnh điều khiển khi bấm nút
function sendControl(state) {
    let payload = JSON.stringify({ "led1": state });
    let message = new Paho.MQTT.Message(payload);
    message.destinationName = TOPIC_SET;
    client.send(message);
    console.log("Đã gửi lệnh: " + payload);
}