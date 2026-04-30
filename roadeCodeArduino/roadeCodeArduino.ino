/*
  Road-E Project - Electronics
  author: Ariel Gal
  date: 30-04-2026

  Changes As Date 30-04:
  1. THE CODE IS FINALLY DONE
  
*/

// Libraries
#include <Wire.h>               // Apply I2C
#include <WiFi.h>               // WIFI
#include <Adafruit_AHTX0.h>     // AHT10
#include <ESP32Servo.h>         // Motor Servo
#include <Adafruit_GFX.h>       // Graphic Screen
#include <Adafruit_SH110X.h>    // Graphic Screen
#include <Adafruit_NeoPixel.h>  // NeoLed
#include <ESPAsyncWebServer.h>  // Host Web Site On ESP

// --- Placeholder HTML for Web Server ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Road-E Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Road-E Control Panel</h1>
  <p>System is online.</p>
</body>
</html>
)rawliteral";
// ---------------------------------------

// WiFi Settings
const char *ssid = "d";
const char *password = "arch8912";
AsyncWebServer server(80);

// I2C Defines
#define i2c_Address 0x3c  // i2c for screen and AHT10

// --- MOTOR PINS ---
#define Motor_Enable 5   
#define Motor1_Pin1 12
#define Motor1_Pin2 13
#define Motor2_Pin1 25     
#define Motor2_Pin2 4   

// PWM Settings
#define Frequency 30000
const int resolution = 8;
const int motorSpeed = 80;  
const int pwmChannel = 0;

// OLED Screen Defines
#define Screen_Width 128
#define Screen_Height 64
#define OLED_Reset -1
Adafruit_SH1106G display = Adafruit_SH1106G(Screen_Width, Screen_Height, &Wire, OLED_Reset);

// NeoLED Defines
#define NeoLed_Pin 14     
#define NumPixels 8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NumPixels, NeoLed_Pin, NEO_GRB + NEO_KHZ800);

// HC-SR04
#define Echo_Pin 33
#define Trig_Pin 32

// IR Sensors
#define IR_Left 36
#define IR_Right 35

// AHT10
Adafruit_AHTX0 aht;

// Servo
Servo myServo;
#define Servo_Pin 18

// LDR
#define LDR 34

// Counters, Checkers & Global States
int angle = 180;                   // check for angle of servo
volatile bool isAutoMode = false;  // switcher between AUTO to MANUAL mode
int servoStep = -2;                // stepper for servo rotate
float h = 0.0;                     // for humidity
float t = 0.0;                     // for temperature
const int black = 1;
const int white = 0;

// Function Declarations
void DisplayMessage(String row1, String row2);
int getAngle(int currAngle);
char getIrDirection(int left, int right);
void setColor(int red, int green, int blue);
void moveForward();
void turnRight();
void turnLeft();
void stopCar();
void moveBackward();

void setup() {
  Serial.begin(9600);

  // IR
  pinMode(IR_Left, INPUT);
  pinMode(IR_Right, INPUT);

  // HC-SR04
  pinMode(Echo_Pin, INPUT);
  pinMode(Trig_Pin, OUTPUT);
/*
  // Servo
  myServo.attach(Servo_Pin);
  myServo.write(0);
  delay(500);
  myServo.write(180);
  delay(500);
  myServo.write(90);
*/

  // OLED
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);

  // LDR
  pinMode(LDR, INPUT);

  // AHT10
  if (!aht.begin()) {
    Serial.println("Could Not Find AHT10 Sensor!");
    while (1) delay(10);
  }

  // Neo Led
  pixels.begin();
  pixels.clear();
  pixels.show();

  // Connect Motors
  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
  pinMode(Motor2_Pin2, OUTPUT);
  
  // Set PWM
  ledcSetup(pwmChannel, Frequency, resolution);
  ledcAttachPin(Motor_Enable, pwmChannel);
  stopCar(); // Ensure car is stopped on boot

  // WiFi Connection
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  DisplayMessage("IP Address: ", WiFi.localIP().toString());

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    String json = "{\"status\":\"Active\",\"temp\":" + String(t) + ",\"hum\":" + String(h) + ",\"lat\":32.0853,\"lng\":34.7818}";
    request->send(200, "application/json", json);
  });

  // Handle Mode Toggle
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("auto")) {
      String autoParam = request->getParam("auto")->value();
      autoParam.toLowerCase();

      isAutoMode = (autoParam == "true" || autoParam == "1");

      stopCar();  // Always stop safely when switching modes
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
      return;
    }

    if (request->hasParam("command")) {
      String cmd = request->getParam("command")->value();
      DisplayMessage("Direction: ", cmd);

      if (cmd == "F") moveForward();
      else if (cmd == "B") moveBackward();
      else if (cmd == "R") turnRight();
      else if (cmd == "L") turnLeft();
      else stopCar();

      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing Command");
    }
  });

  server.begin();
}

void loop() {
  // Get Temperature & Humidity
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  t = temp.temperature;
  h = humidity.relative_humidity;

  int light = analogRead(LDR);

  // Control LEDs based on light
  if (light >= 2000) {
    setColor(100, 100, 100);
  } else {
    setColor(0, 0, 0);
  }

  int leftSide = digitalRead(IR_Left);
  int rightSide = digitalRead(IR_Right);

  // Start rotating the servo
  //myServo.write(angle);
  //angle = getAngle(angle);
  //delay(50); // Reduced delay for smoother multitasking

  if (isAutoMode) {
    char drive = getIrDirection(leftSide, rightSide);
    switch (drive) {
      case 'F': moveForward(); break;
      case 'L': turnLeft(); break;
      case 'R': turnRight(); break;
      case 'S': stopCar(); break;
      case 'B': moveBackward(); break;
    }
  }
}

void DisplayMessage(String row1, String row2) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 2);
  display.print(row1);
  display.setCursor(5, 32);
  display.print(row2);
  display.display();
}

int getAngle(int currAngle) {
  if (currAngle >= 180) servoStep = -2;
  if (currAngle <= 0) servoStep = 2;
  return currAngle + servoStep;
}

char getIrDirection(int left, int right) {
  if (left == black && right == white) return 'L';
  if (left == black && right == black) return 'F';
  if (left == white && right == black) return 'R';
  return 'S';
}


void setColor(int red, int green, int blue) {
  pixels.clear();
  for (int i = 0; i < NumPixels; i++) {
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
  }
  pixels.show();
}

void moveForward() {
  ledcWrite(Motor_Enable, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
}

void moveBackward() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, HIGH);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, HIGH);
}

void turnRight() {
  ledcWrite(Motor_Enable, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
}

void turnLeft() {
  ledcWrite(Motor_Enable, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, LOW);
}

void stopCar() {
  ledcWrite(Motor_Enable, 0);
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, LOW);
}
