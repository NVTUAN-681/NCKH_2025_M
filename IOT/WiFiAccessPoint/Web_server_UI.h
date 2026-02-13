// web_server_ui.h
const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Control Center</title>
  <link rel="stylesheet" href="https://your-link.com/style.css"> 
</head>
<body>
  <h1>Dieu khien LED</h1>
  <button onclick="sendRequest('/led1/on')">Bat LED 1</button>
  <button onclick="sendRequest('/led1/off')">Tat LED 1</button>

  <script>
    function sendRequest(path) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", path, true);
      xhr.send();
    }
  </script>
</body>
</html>)rawliteral";