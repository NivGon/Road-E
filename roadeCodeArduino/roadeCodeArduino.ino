/*
  Road-E Project - Electronics
  
  author: Ariel Gal
  date: 05-02-2026

  Changes In Code At Date 05-02:
  1. Add code for L293D to drive forword
  2. Start on code for all project (in arduino anly, without WS & SQL/Python)
*/

//Libraries
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

//total defines
#define i2c_Address 0x3c  //i2c for screen and AHT10
#define Motor_Enable 14   //Motor Enable Pin

//Motor 1 Pins - Left Side
#define Motor1_Pin1 12
#define Motor1_Pin2 13

//Motor 2 Pins - Left Side
#define Motor2_Pin1 25
#define Motor2_Pin2 4

//PWM Settings
#define Frequancy 30000
const int resolution = 8;    //Range between 0-255
const int motorSpeed = 130;  //High speed to ensure movement

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

//counters and checkers
int angle = 180;  //check for angle of servo
int direction = -1;

void setup() {
  Serial.begin(9600);

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
  ledcAttach(Motor_Enable, Frequancy, resolution);
  ledcAttach(Motor_Enable, Frequancy, resolution);

  //OLED
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
  display.setTextColor(SH110X_WHITE, SH110X_BLACK);  //(0,1)

  //aht10
  if (!aht.begin()) {
    Serial.println("Could Not Find AHT10 Sensor!");
    while (1) delay(10);
  }
}

void loop() {
  //Get Temperature & Humidity
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  //Check for light (from LDR) to turn on lights
  // if (analogRead(LDR) <= 2000) {
  //   digitalWrite(Red, HIGH);
  //   digitalWrite(Green, HIGH);
  //   digitalWrite(Blue, HIGH);
  // } else {
  //   digitalWrite(Red, LOW);
  //   digitalWrite(Green, LOW);
  //   digitalWrite(Blue, LOW);
  // }

  //Start rotating the servo
  myServo.write(angle);
  direction = getDirection(angle, direction);

  //Check to where rotate the servo
  switch (direction) {
    case 1:
      angle++;
      break;
    case -1:
      angle--;
      break;
  }

  //Set direction of car forword
  Serial.println("Moving Forward");
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
  ledcWrite(Motor_Enable, motorSpeed);
  delay(500);

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
