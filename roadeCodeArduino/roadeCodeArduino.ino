/*
  Road-E Project - Electronics
  
  author: Ariel Gal
  date: 25-02-2026

  Changes In Code At Date 25-02:
  1. add code to drive the car via the website


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

//total defines
#define i2c_Address 0x3c  //i2c for screen and AHT10
#define Motor_Enable 14   //Motor Enable Pin

//Motor 1 Pins - Left Side
#define Motor1_Pin1 12
#define Motor1_Pin2 13

//Motor 2 Pins - Right Side
#define Motor2_Pin1 25
#define Motor2_Pin2 4

//PWM Settings
#define Frequency 30000
const int resolution = 8;    //Range between 0-255
const int motorSpeed = 100;  //High speed to ensure movement

//defines for OLED screen
#define Screen_Width 128  //OLED Display In WIdth (in pixels)
#define Screen_Height 64  //OLED Display In Height (in pixles)
#define OLED_Reset -1
Adafruit_SH1106G display = Adafruit_SH1106G(Screen_Width, Screen_Height, &Wire, OLED_Reset);

//defines for NeoLED
#define Pin 14
#define NumPixels 8
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NumPixels, Pin, NEO_GRB + NEO_KHZ800);

//hcsr04
#define Echo_Pin 33
#define Trig_Pin 32

//IR
#define IR2 36  //Left Side - Connect to Pin 36
#define IR4 35  //Right Side - Connect to Pin 35

//aht10
Adafruit_AHTX0 aht;

//Servo
Servo myServo;
#define Servo_Pin 18

//LDR
#define LDR 34

//RGB
#define Red 26
#define Green 27
#define Blue 2

//WIFI
const char* ssid = "Road-E";
const char* password = "roade1234";
AsyncWebServer server(80);

//counters and checkers
int angle = 180;  //check for angle of servo
int servoDirection = 1;
const int black = 1;
const int white = 0;

void setup() {
  Serial.begin(9600);

  //hcsr
  pinMode(Echo_Pin, INPUT);
  pinMode(Trig_Pin, OUTPUT);

  //IR
  pinMode(IR4, INPUT);
  pinMode(IR2, INPUT);

  //Servo
  myServo.attach(Servo_Pin);
  myServo.write(0);
  delay(500);
  myServo.write(180);
  delay(500);
  myServo.write(90);

  //Neo Led
  pixels.begin();
  pixels.clear();

  //RGB
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);

  // Connect Motors
  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
  pinMode(Motor2_Pin2, OUTPUT);

  // Attach PWM
  if (!ledcAttach(Motor_Enable, Frequency, resolution)) {
    Serial.println("PWM Setup Failed!");
  }

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

  //WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nRoad-E Online!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  
}

void loop() {
  //Get Temperature & Humidity
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  //Check for light (from LDR) to turn on lights
  int light = analogRead(LDR);
  if (light <= 2000) {
    digitalWrite(Red, HIGH);
    digitalWrite(Green, HIGH);
    digitalWrite(Blue, HIGH);
  } else {
    digitalWrite(Red, LOW);
    digitalWrite(Green, LOW);
    digitalWrite(Blue, LOW);
  }

  //Start rotating the servo
  myServo.write(angle);
  servoDirection = getDirection(angle, servoDirection);

  //Check to where rotate the servo
  switch (servoDirection) {
    case 1:
      angle++;
      break;
    case -1:
      angle--;
      break;
  }

  //Drive the car
  int leftReading = digitalRead(IR2);   // Pin 36
  int rightReading = digitalRead(IR4);  // Pin 35
  char directCar = direction(leftReading, rightReading);

  switch (directCar) {
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

  delay(50);

  myServo.write(angle);
}

void DisplayMessage(String row1, String row2) {  //function to print messages on OLED screen
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(50, 10);
  display.print(row1);
  display.setCursor(35, 35);
  display.print(row2);
  display.display();
}

int getDirection(int currentAngle, int currentDir) {  //function to check direction of servo and reset direcrion
  if (currentAngle >= 180) return -1;
  if (currentAngle <= 0) return 1;
  return currentDir;
}

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

void moveForward() {
  // Set Speed
  ledcWrite(Motor_Enable, motorSpeed);

  // Motor 1 Forward
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);

  // Motor 2 Forward
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, HIGH);
}

void turnRight() {
  // To turn Right: Keep Left motor moving, STOP Right motor
  ledcWrite(Motor_Enable, motorSpeed);

  // Left Motor Forward
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, LOW);

  // Right Motor Stop (Pivot turn)
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, HIGH);
}

void turnLeft() {
  // To turn Left: Keep Right motor moving, STOP Left motor
  ledcWrite(Motor_Enable, motorSpeed);

  // Left Motor Stop (Pivot turn)
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);

  // Right Motor Forward
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, LOW);
}

void stopCar() {
  ledcWrite(Motor_Enable, 0);  // Speed 0

  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, LOW);
}
