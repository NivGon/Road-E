#include <WiFi.h>
#include <ESPAsyncWebServer.h>

// --- WIFI CONFIG ---
const char* ssid = "d";
const char* password = "arch8912";

// --- MOTOR PINS ---
#define Motor1_Pin1 12
#define Motor1_Pin2 13
#define Motor2_Pin1 4
#define Motor2_Pin2 25
#define Motor_Enable 14

#define IR2 36  
#define IR4 35  

const int freq = 30000;
const int pwmChannel = 0;
const int resolution = 8;
const int motorSpeed = 180; 

AsyncWebServer server(80);

void stopCar() {
  int leftReading = digitalRead(IR2);   // Pin 36
  int rightReading = digitalRead(IR4);  // Pin 35

  char direction(int left, int right) {
  if (left == 1 && right == 0) {
    return 'l';
  }
  if (left == 1 && right == 1) {
    return 'f';
  }
  if (left == 0 && right == 1) {
    return 'r';
  }
  return 's';
  }

  switch (direction) {
    case 's':
      stopCar();
      break;
    case 'f':
      moveForward();
      break;
    case 'r':
      turnRight();
      break;
    case 'l':
      turnLeft();
      break;
  }

}

void moveForward() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, LOW);  digitalWrite(Motor2_Pin2, HIGH);
}

void moveBackward() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, HIGH); digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, HIGH); digitalWrite(Motor2_Pin2, LOW);
}

void turnLeft() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, HIGH); digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);  digitalWrite(Motor2_Pin2, HIGH);
}

void turnRight() {
  ledcWrite(pwmChannel, motorSpeed);
  digitalWrite(Motor1_Pin1, LOW);  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, HIGH); digitalWrite(Motor2_Pin2, LOW);
}
void stopCar() {
  ledcWrite(Motor_Enable, 0);  // Speed 0
  digitalWrite(Motor1_Pin1, LOW); digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW); digitalWrite(Motor2_Pin2, LOW);
}

void setup() {
  Serial.begin(115200);

  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
  pinMode(Motor2_Pin2, OUTPUT);

  pinMode(IR4, INPUT);
  pinMode(IR2, INPUT);

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

  // הגדרת CORS גלובלית - חייב להופיע לפני ה-Routes
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, OPTIONS");
  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "*");

  // טיפול בבקשות OPTIONS (Preflight) שהדפדפן שולח
  server.onNotFound([](AsyncWebServerRequest *request) {
    if (request->method() == HTTP_OPTIONS) {
      request->send(200);
    } else {
      request->send(404);
    }
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"status\":\"Active\",\"temp\":25,\"hum\":45,\"lat\":32.0853,\"lng\":34.7818}";
    request->send(200, "application/json", json);
  });

  server.on("/drive", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("command")) {
      String cmd = request->getParam("command")->value();
      if (cmd == "F") moveForward();
      else if (cmd == "B") moveBackward();
      else if (cmd == "L") turnLeft();
      else if (cmd == "R") turnRight();
      else stopCar(); 
      
      request->send(200, "text/plain", "OK");
    } else {
      request->send(400, "text/plain", "Missing Command");
    }
  });

  server.begin();
}

void loop() {}
