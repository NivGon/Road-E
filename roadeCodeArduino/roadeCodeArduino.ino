/*
  Road-E Project - Electronics
  author: Ariel Gal
  date: 30-03-2026

  Changes As Date 30-03:
  1. code ready
  2. need to add code for camera and GPS when get ones
  
*/

//Libraries
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncWebServer.h>

// --- Placeholder HTML for Web Server ---
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>Road-E Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
</head>
<body>
  <h1>Road-E Control Panel</h1>
  <p>System is online. Replace this with your actual UI.</p>
</body>
</html>
)rawliteral";
// ---------------------------------------

//WiFi Settings
const char *ssid = "WIFI_NAME";
const char *password = "WIFI_PASSWORD";
AsyncWebServer server(80);

//total defines
#define i2c_Address 0x3c  //i2c for screen and AHT10
#define Motor_Enable 5    //Motor Enable Pin

//Motor 1 Pins - Controls Left Side
#define Motor1_Pin1 12
#define Motor1_Pin2 13

//Motor 2 Pins - Controls Right Side
#define Motor2_Pin1 25
#define Motor2_Pin2 4

//PWM Settings
#define Frequency 30000
const int resolution = 8;   //Range between 0-255
const int motorSpeed = 80;  //High speed to ensure movement
const int pwmChannel = 0;

//defines for OLED screen
#define Screen_Width 128  //OLED Display In Width (in pixels)
#define Screen_Height 64  //OLED Display In Height (in pixles)
#define OLED_Reset -1
Adafruit_SH1106G display = Adafruit_SH1106G(Screen_Width, Screen_Height, &Wire, OLED_Reset);

//defines for NeoLED
#define NeoLed_Pin 14
#define NumPixels 8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NumPixels, NeoLed_Pin, NEO_GRB + NEO_KHZ800);

//hcsr04
#define Echo_Pin 33
#define Trig_Pin 32

//IR
#define IR_Left 36
#define IR_Right 35

//aht10
Adafruit_AHTX0 aht;

//Servo
Servo myServo;
#define Servo_Pin 18

//LDR
#define LDR 34

//Counters and Checkers
int angle = 180;  //check for angle of servo
volatile bool isAutoMode = false;
int servoStep = -2;
float h = 0.0;
float t = 0.0;

// Function Declarations (Best practice to declare them before using them)
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

  //IR
  pinMode(IR_Left, INPUT);
  pinMode(IR_Right, INPUT);

  //hcsr
  pinMode(Echo_Pin, INPUT);
  pinMode(Trig_Pin, OUTPUT);

  //Servo
  myServo.attach(Servo_Pin);
  myServo.write(0);
  delay(500);
  myServo.write(180);
  delay(500);
  myServo.write(90);

  //OLED
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);  //(0,1)

  //LDR
  pinMode(LDR, INPUT);

  //aht10
  if (!aht.begin()) {
    Serial.println("Could Not Find AHT10 Sensor!");
    while (1) delay(10);
  }

  //Neo Led
  pixels.begin();
  pixels.clear();
  pixels.show();  // Ensure they start off

  // Connect Motors
  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
  pinMode(Motor2_Pin2, OUTPUT);

  // Set PWM
  ledcSetup(pwmChannel, Frequency, resolution);
  ledcAttachPin(Motor_Enable, pwmChannel);

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

  // FIXED: IPAddress to String conversion
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
    // FIXED: Corrected JSON formatting and Float to String conversion
    String json = "{\"status\":\"Active\",\"temp\":" + String(t) + ",\"hum\":" + String(h) + ",\"lat\":32.0853,\"lng\":34.7818}";
    request->send(200, "application/json", json);
  });

  // Handle Mode Toggle
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("auto")) {
      String autoParam = request->getParam("auto")->value();
      autoParam.toLowerCase();  // Make it case-insensitive

      // Accept "true" or "1" just in case the frontend formats it differently
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
    // 1. Block manual commands if we are in auto mode
    if (isAutoMode) {
      request->send(403, "text/plain", "Ignored: Car is in Auto Mode");
      return;
    }

    // 2. Execute manual commands
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
  //Get Temperature & Humidity
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);
  t = temp.temperature;
  h = humidity.relative_humidity;

  int light = analogRead(LDR);

  // FIXED: Added else block to turn lights off
  if (light >= 2000) {
    setColor(100, 100, 100);
  } else {
    setColor(0, 0, 0);
  }

  int leftSide = digitalRead(IR_Left);
  int rightSide = digitalRead(IR_Right);

  //Start rotating the servo
  myServo.write(angle);
  angle = getAngle(angle);
  delay(150);

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

void DisplayMessage(String row1, String row2) {  //function to print messages on OLED screen
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(5, 2);
  display.print(row1);
  display.setCursor(5, 32);
  display.print(row2);
  display.display();
}

int getAngle(int currAngle) {  //function to get the servo angle
  if (currAngle >= 180) servoStep = -2;
  if (currAngle <= 0) servoStep = 2;
  return currAngle + servoStep;
}

char getIrDirection(int left, int right) {  //function to set the direction of the car via the IR
  if (left == 1 && right == 0) return 'L';
  if (left == 1 && right == 1) return 'F';
  if (left == 0 && right == 1) return 'R';
  return 'S';
}

void setColor(int red, int green, int blue) {  //function to set and show the NeoLed
  pixels.clear();
  for (int i = 0; i < NumPixels; i++) {
    pixels.setPixelColor(i, pixels.Color(red, green, blue));
  }
  pixels.show();
}



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
