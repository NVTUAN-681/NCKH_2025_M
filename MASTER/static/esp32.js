
// nói qua về DOM (document) nó là cánh cổng để liên kết .html với .js, ở .js ta có thể truy cập được cả style của trang web thông qua DOM

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

// 4. Hàm xử lý sự kiện
function onFail() {
    console.log("Không thể kết nối HiveMQ!");
    document.getElementById("connection-status").innerText = "Can't connect to Cloud";
    document.getElementById("connection-status").style.color = "red";
}
function onConnect() {
    console.log("Đã kết nối HiveMQ!");
    document.getElementById("connection-status").innerText = "Cloud Connected";
    document.getElementById("connection-status").style.color = "green";
    // Đăng ký nhận trạng thái từ ESP32
    client.subscribe(TOPIC_STATUS);
    console.log("Đã Subscribe thành công topic: data");
}

function onConnectionLost(responseObject) {
    if (responseObject.errorCode !== 0) {
        console.log("Mất kết nối MQTT: " + responseObject.errorMessage);
        document.getElementById("connection-status").innerText = "Mất kết nối Cloud";
        document.getElementById("connection-status").style.color = "orange";
    }
}


function toggleCamera(isOn) {
        const streamImg = document.getElementById('streamFrame'); //getElementById() :lấy id của tag
        const overlay = document.getElementById('camOverlay'); // overlay offcam
        if (isOn) {
            console.log("Camera: Starting Stream..."); // in thông tin lên console
            // Thay URL IP cua ESP32-CAM
            // streamImg.src = "http://192.168.1.100:81/stream"; 
            streamImg.src = "/video_feed";
            streamImg.style.display = "block"; /* mo hinh anh camera*/
            overlay.style.display = "none"; /*tat overlay offcam */
        } else {
            console.log("Camera: Stopping Stream..."); 
            streamImg.src = ""; //ngat ket noi cam
            streamImg.style.display = "none"; 
            overlay.style.display = "block";
        }
    }

function onMessageArrived(message) {
    console.log("Dữ liệu về: " + message.payloadString);
    try {
        let data = JSON.parse(message.payloadString);
        
        // Cập nhật cho Đèn 1
        if (data.hasOwnProperty('Living_light')) {
            const icon = document.getElementById('icon-Living_light');
            if (data.Living_light === 1) {
                icon.classList.add('light-on'); // Thêm hiệu ứng sáng
                console.log("Giao diện: Đèn 1 Bật");
            } else {
                icon.classList.remove('light-on'); // Tắt hiệu ứng sáng
                console.log("Giao diện: Đèn 1 Tắt");
            }
        }

        if (data.hasOwnProperty('Kitchen_light')) {
            const icon = document.getElementById('icon-Kitchen_light');
            if (data.Kitchen_light === 1) {
                icon.classList.add('light-on'); // Thêm hiệu ứng sáng
                console.log("Giao diện: Đèn 2 Bật");
            } else {
                icon.classList.remove('light-on'); // Tắt hiệu ứng sáng
                console.log("Giao diện: Đèn 2 Tắt");
            }
        }

        // Cập nhật cho Cửa (Nếu AI của bạn có điều khiển cửa)
        if (data.hasOwnProperty('Door')) {
            const iconDoor = document.getElementById('icon-Door');
            if (data.Door === 1) {
                iconDoor.classList.add('door-open');
                iconDoor.classList.replace('fa-door-closed', 'fa-door-open');
            } else {
                iconDoor.classList.remove('door-open');
                iconDoor.classList.replace('fa-door-open', 'fa-door-closed');
            }
        }
    } catch (e) {
        console.log("Lỗi xử lý dữ liệu giao diện: " + e);
    }
}

// hàm này dùng để gửi lệnh từ web đến ESP32 thông qua MQTT
 function animateDevice(device, action) {
    console.log(`Sending command: ${device} -> ${action}`);

    // 1. Tạo dữ liệu để gửi đi
    let command = {};
    if (device === 'Living_light') command = { Living_light: action === 'ON' ? 1 : 0 };
    if (device === 'Kitchen_light') command = { Kitchen_light: action === 'ON' ? 1 : 0 };
    if (device === 'Door') command = { Door: action === 'OPEN' ? 1 : 0 };

    // 2. Gửi dữ liệu lên MQTT Topic "data"
    const message = new Paho.MQTT.Message(JSON.stringify(command));
    message.destinationName = TOPIC_SET;
    client.send(message);

    // 3. Hiệu ứng giao diện (giữ nguyên logic cũ của bạn)
    if (device === 'Living_light'){
        const icon = document.getElementById('icon-Living_light');
        if (action === 'ON') icon.classList.add('light-on');
        else icon.classList.remove('light-on');
    }
    if (device === 'Kitchen_light'){
        const icon = document.getElementById('icon-Kitchen_light');
        if (action === 'ON') icon.classList.add('light-on');
        else icon.classList.remove('light-on');
    }
    if(device === 'Door'){
        const iconDoor = document.getElementById('icon-Door');
        if (action === 'OPEN') {
            iconDoor.classList.add('door-open');
            iconDoor.classList.replace('fa-door-closed', 'fa-door-open');
        } else {
            iconDoor.classList.remove('door-open');
            iconDoor.classList.replace('fa-door-open', 'fa-door-closed');
        }
        }
}

// Thêm các biến quản lý timer vào đầu file esp32.js
// Biến toàn cục để quản lý tiến trình
// Đối tượng lưu trữ lịch trình của tất cả thiết bị
let deviceSchedules = {
    'Living_light': { interval: null, startTime: null, endTime: null, isStarted: false },
    'Kitchen_light': { interval: null, startTime: null, endTime: null, isStarted: false },
    'Door': { interval: null, startTime: null, endTime: null, isStarted: false }
};


// Hàm này để đảm bảo khi bạn chuyển sang thiết bị khác, 
// nếu thiết bị đó chưa có lịch thì nút Hủy phải ẩn đi
function onDeviceChange() {
    const device = document.getElementById('timerDevice').value;
    const schedule = deviceSchedules[device];

    document.getElementById('startTime').value = schedule.startTime || "";
    document.getElementById('endTime').value = schedule.endTime || "";

    if (schedule.interval) {
        document.getElementById('cancelTimerBtn').classList.remove('d-none');
        // Cập nhật lại câu thông báo nếu cần
        if (schedule.isStarted) {
            document.getElementById('timerStatus').innerText = `Đang chạy lịch trình cho ${device}`;
        } else {
            document.getElementById('timerStatus').innerText = `Đã đặt lịch chờ cho ${device}`;
        }
    } else {
        document.getElementById('countdown').innerText = "00:00:00";
        document.getElementById('timerStatus').innerText = "Chưa có lịch trình nào được thiết lập";
        document.getElementById('cancelTimerBtn').classList.add('d-none');
    }
}

function setSchedule() {
    const device = document.getElementById('timerDevice').value;
    const startStr = document.getElementById('startTime').value;
    const endStr = document.getElementById('endTime').value;

    if (!startStr || !endStr) {
        alert("Vui lòng nhập đầy đủ thời gian!");
        return;
    }

    // 1. Xóa lịch trình cũ CỦA RIÊNG THIẾT BỊ NÀY nếu đang chạy
    if (deviceSchedules[device].interval) {
        clearInterval(deviceSchedules[device].interval);
    }

    // 2. Lưu thông số mới vào Object
    deviceSchedules[device].startTime = startStr;
    deviceSchedules[device].endTime = endStr;
    deviceSchedules[device].isStarted = false;

    const getFullDate = (timeStr) => {
        const [h, m] = timeStr.split(':').map(Number);
        const d = new Date();
        d.setHours(h, m, 0, 0);
        return d;
    };

    let startTime = getFullDate(startStr);
    let endTime = getFullDate(endStr);
    if (endTime <= startTime) endTime.setDate(endTime.getDate() + 1);

    document.getElementById('cancelTimerBtn').classList.remove('d-none');

    // 3. Chạy vòng lặp riêng cho thiết bị này
    deviceSchedules[device].interval = setInterval(() => {
        const now = new Date();
        const currentSelectedDevice = document.getElementById('timerDevice').value;

        // Logic điều khiển
        if (now >= startTime && now < endTime) {
            if (!deviceSchedules[device].isStarted) {
                animateDevice(device, 'ON');
                deviceSchedules[device].isStarted = true;
            }
            // Chỉ cập nhật giao diện nế người dùng ĐANG CHỌN thiết bị này trên màn hình
            if (currentSelectedDevice === device) {
                displayTime(endTime - now, `Đèn ${device} đang bật (đếm ngược tắt)`);
            }
        } 
        else if (now >= endTime) {
            animateDevice(device, 'OFF');
            stopDeviceSchedule(device);
        } 
        else {
            if (currentSelectedDevice === device) {
                displayTime(startTime - now, `Chờ đến lúc bật ${device}`);
            }
        }
    }, 1000);
}

function stopDeviceSchedule(device) {
    if (deviceSchedules[device].interval) {
        clearInterval(deviceSchedules[device].interval);
        deviceSchedules[device].interval = null;
        deviceSchedules[device].startTime = null;
        deviceSchedules[device].endTime = null;
        
        // Nếu đang nhìn đúng thiết bị này thì reset màn hình
        if (document.getElementById('timerDevice').value === device) {
            cancelTimerUI();
        }
    }
}

// Hàm bổ trợ xóa giao diện
// Hàm reset giao diện hiển thị đếm ngược
function cancelTimerUI() {
    document.getElementById('countdown').innerText = "00:00:00";
    document.getElementById('timerStatus').innerText = "Chưa có lịch trình nào được thiết lập";
    document.getElementById('cancelTimerBtn').classList.add('d-none');
}

// Hàm gọi từ nút "Hủy bỏ" trên giao diện
function cancelCurrentTimer() {
    const device = document.getElementById('timerDevice').value;
    const schedule = deviceSchedules[device];

    if (schedule.interval) {
        // 1. Dừng vòng lặp đếm ngược
        clearInterval(schedule.interval);

        // 2. Kiểm tra nếu đèn ĐANG BẬT thì phải tắt đi
        if (schedule.isStarted) {
            console.log(`Lịch trình đang chạy, tiến hành tắt thiết bị: ${device}`);
            // Gửi lệnh tắt tương ứng (OFF cho đèn, CLOSE cho cửa)
            const offAction = (device === 'Door') ? 'CLOSE' : 'OFF';
            animateDevice(device, offAction);
        }

        // 3. Reset dữ liệu của thiết bị trong bộ nhớ
        schedule.interval = null;
        schedule.startTime = null;
        schedule.endTime = null;
        schedule.isStarted = false;

        // 4. Reset giao diện nhập liệu về mặc định (Trống)
        document.getElementById('startTime').value = "";
        document.getElementById('endTime').value = "";

        // 5. Reset khu vực hiển thị đếm ngược (Cột bên phải)
        document.getElementById('countdown').innerText = "00:00:00";
        document.getElementById('timerStatus').innerText = "Chưa có lịch trình nào được thiết lập";
        document.getElementById('cancelTimerBtn').classList.add('d-none');

        console.log(`Đã hủy bỏ và reset hoàn toàn cho: ${device}`);
    }
}

// Hàm hiển thị thời gian đếm ngược lên màn hình
function displayTime(diffMs, statusText) {
    if (diffMs < 0) return;
    const h = Math.floor(diffMs / 3600000).toString().padStart(2, '0');
    const m = Math.floor((diffMs % 3600000) / 60000).toString().padStart(2, '0');
    const s = Math.floor((diffMs % 60000) / 1000).toString().padStart(2, '0');

    document.getElementById('countdown').innerText = `${h}:${m}:${s}`;
    document.getElementById('timerStatus').innerText = statusText;
}