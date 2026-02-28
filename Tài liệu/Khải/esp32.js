
// nói qua về DOM (document) nó là cánh cổng để liên kết .html với .js, ở .js ta có thể truy cập được cả style của trang web thông qua DOM

function toggleCamera(isOn) {
        const streamImg = document.getElementById('streamFrame'); //getElementById() :lấy id của tag
        const overlay = document.getElementById('camOverlay'); // overlay offcam

        if (isOn) {
            console.log("Camera: Starting Stream..."); // in thông tin lên console
            // Thay URL IP cua ESP32-CAM
            streamImg.src = "http://192.168.1.100:81/stream"; 
            streamImg.style.display = "block"; /* mo hinh anh camera*/
            overlay.style.display = "none"; /*tat overlay offcam */
        } else {
            console.log("Camera: Stopping Stream..."); 
            streamImg.src = ""; //ngat ket noi cam
            streamImg.style.display = "none"; 
            overlay.style.display = "block";
        }
    }

    function animateDevice(device, action) {
        console.log(`Sending command: ${device} -> ${action}`); // in thông tin lên console

        // light 1
        if (device === 'light1') { // "=== stricly equally thường dùng trong .js tức là phải cùng kiểu, cùng giá trị
            const icon = document.getElementById('icon-light1');// lấy class giao diện cái bóng đèn
            if (action === 'ON') icon.classList.add('light-on'); // classList.add / classList.remove tức là add class giao diện đã tạo ở .css /bật thì add hiệu ứng .light-on{} và remove khi tắt
            else icon.classList.remove('light-on');
        }

        // light 2
        if (device === 'light2') {
            const icon = document.getElementById('icon-light2');
            if (action === 'ON') icon.classList.add('light-on');
            else icon.classList.remove('light-on');
        }

        // 3. door(motor)
        if (device === 'door') {
            const icon = document.getElementById('icon-door');
            if (action === 'OPEN') {
                icon.classList.add('door-open');
                icon.classList.replace('fa-door-closed', 'fa-door-open');// replace tức là thay thế class cho class hiện tại
            } else {
                icon.classList.remove('door-open');
                icon.classList.replace('fa-door-open', 'fa-door-closed');  // fa-door-open và fa-door-closed lấy từ "Font Awesome" 
            }
        }


    }