#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- WIFI CONFIG ---
const char *ssid = "Gal";
const char *password = "Ydan20190616";

// --- MOTOR PINS ---
#define Motor1_Pin1 12
#define Motor1_Pin2 13
#define Motor2_Pin1 4
#define Motor2_Pin2 25
#define Motor_Enable 14

// --- IR PINS ---
#define IR_Left 36  // Formerly IR2
#define IR_Right 35 // Formerly IR4

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
const int motorSpeed = 180;

AsyncWebServer server(80);

// --- GLOBAL STATE ---
bool isAutoMode = false;

// --- HTML & JS PAYLOAD ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Road-E Controller</title>
<style>
  body { background-color: #121212; color: white; font-family: Arial, sans-serif; display: flex; flex-direction: column; align-items: center; justify-content: center; height: 100vh; margin: 0; user-select: none; }
  .joystick-container { display: grid; grid-template-columns: repeat(3, 70px); grid-template-rows: repeat(3, 70px); gap: 10px; margin-bottom: 30px; }
  .control-btn { background-color: #2a2a2a; border: 1px solid #3a3a3a; border-radius: 8px; color: white; font-size: 24px; cursor: pointer; display: flex; align-items: center; justify-content: center; transition: background-color 0.1s, transform 0.1s; }
  .control-btn:active { background-color: #444; transform: scale(0.95); }
  .btn-up { grid-column: 2; grid-row: 1; }
  .btn-left { grid-column: 1; grid-row: 2; }
  .btn-stop { grid-column: 2; grid-row: 2; color: #ff4d4d; font-weight: bold; font-size: 16px; }
  .btn-right { grid-column: 3; grid-row: 2; }
  .btn-down { grid-column: 2; grid-row: 3; }
  .mode-toggle-btn { background-color: #2a2a2a; border: 1px solid #3a3a3a; border-radius: 8px; color: #4CAF50; font-size: 16px; font-weight: bold; padding: 15px 30px; cursor: pointer; transition: all 0.3s ease; width: 230px; text-align: center; text-transform: uppercase; letter-spacing: 1px; }
  .mode-toggle-btn:active { transform: scale(0.98); }
  .mode-toggle-btn.auto-mode { color: #2196F3; border-color: #2196F3; background-color: rgba(33, 150, 243, 0.1); }
</style>
</head>
<body>

  <div class="joystick-container">
    <button class="control-btn btn-up" onpointerdown="sendCmd('F')" onpointerup="sendCmd('S')">▲</button>
    <button class="control-btn btn-left" onpointerdown="sendCmd('L')" onpointerup="sendCmd('S')">◀</button>
    <button class="control-btn btn-stop" onpointerdown="sendCmd('S')">STOP</button>
    <button class="control-btn btn-right" onpointerdown="sendCmd('R')" onpointerup="sendCmd('S')">▶</button>
    <button class="control-btn btn-down" onpointerdown="sendCmd('B')" onpointerup="sendCmd('S')">▼</button>
  </div>

  <button id="modeToggleBtn" class="mode-toggle-btn">Mode: Manual</button>

  <script>
    const modeBtn = document.getElementById('modeToggleBtn');
    let isAutoMode = false;

    // Send drive commands to ESP
    function sendCmd(cmd) {
      if(!isAutoMode) { // Only send manual commands if not in auto
        fetch(`/drive?command=${cmd}`);
      }
    }

    // Toggle Auto/Manual
    modeBtn.addEventListener('click', () => {
      isAutoMode = !isAutoMode;
      if (isAutoMode) {
        modeBtn.textContent = 'Mode: Auto';
        modeBtn.classList.add('auto-mode');
      } else {
        modeBtn.textContent = 'Mode: Manual';
        modeBtn.classList.remove('auto-mode');
      }
      // Send the new mode to the ESP
      fetch(`/mode?auto=${isAutoMode}`);
    });
  </script>
</body>
</html>
)rawliteral";

// --- MOTOR FUNCTIONS ---
void moveForward() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, HIGH);
}

void moveBackward() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, HIGH);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
}

void turnLeft() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, HIGH);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, HIGH);
}

void turnRight() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
}

void stopCar() {
  ledcWrite(pwmChannel, 0);  // Speed 0
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, LOW);
}

// --- IR LOGIC ---
char getIrDirection(int left, int right) {
  if (left == 1 && right == 0) return 'L';
  if (left == 1 && right == 1) return 'F';
  if (left == 0 && right == 1) return 'R';
  return 'S';
}

void setup() {
  Serial.begin(115200);

  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
  pinMode(Motor2_Pin2, OUTPUT);

  pinMode(IR_Left, INPUT); 
  pinMode(IR_Right, INPUT);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(Motor_Enable, pwmChannel);

  stopCar();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nRoad-E Online!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });

  // Serve the Web Page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"status\":\"Active\",\"temp\":25,\"hum\":45,\"lat\":32.0853,\"lng\":34.7818}";
    request->send(200, "application/json", json);
  });

  // Handle Mode Toggle
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("auto")) {
      String autoParam = request->getParam("auto")->value();
      isAutoMode = (autoParam == "true");
      stopCar(); // Always stop safely when switching modes
      Serial.println(isAutoMode ? "Switched to AUTO" : "Switched to MANUAL");
      request->send(200, "text/plain", "Mode updated");
    } else {
      request->send(400, "text/plain", "Missing auto parameter");
    }
  });

  // Handle Manual Drive Commands
  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isAutoMode) {
      request->send(403, "text/plain", "Ignored: Car is in Auto Mode");
      return; // Do nothing if in auto mode
    }

    if (request->hasParam("command")) {
      String cmd = request->getParam("command")->value();
      if (cmd == "F") moveForward();
      else if (cmd == "B") moveBackward();
      else if (cmd == "R") turnLeft();  
      else if (cmd == "L") turnRight(); 
      else stopCar();

      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing Command");
    }
  });

  server.begin();
}

void loop() {
  if (isAutoMode) {
    // Read the newly named IR sensors
    int leftIR_val = digitalRead(IR_Left); 
    int rightIR_val = digitalRead(IR_Right);

    // Get the required direction
    char dir = getIrDirection(leftIR_val, rightIR_val);

    // Execute the movement
    if (dir == 'F') moveForward();
    else if (dir == 'L') turnLeft();
    else if (dir == 'R') turnRight();
    else stopCar();

    delay(50); // Small delay to prevent overwhelming the motor driver
  }
}
