/*
  Road-E Project - Electronics
  
  author: Ariel Gal
  date: 04-02-2026

  Changes In Code At Date 04-02:
  1. add libraries that needed
  2. declare settings and pins
  3. start basics declerations for the loop code
  4. add function to print on OLED screen
  5. add function to check and reset the servo
  
*/

//Libraries
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ESP32Servo.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

//total defines
#define i2c_Address 0x3c

//defines for OLED screen
#define Screen_Width 128  //OLED Display In WIdth (in pixels)
#define Screen_Height 64  //OLED Display In Height (in pixles)
#define OLED_Reset -1
Adafruit_SH1106G display = Adafruit_SH1106G(Screen_Width, Screen_Height, &Wire, OLED_Reset);

//defines for NeoLED
#define Pin 14
#define NumPixels 8
Adafruit_NeoPixel pixels = Adafruit_NeoPixels(NumPixels, PIn, NEO_RGB, NEO_KHZ800);

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
const int angle = 180;

void setup() {
  Serial.begin(9600)

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
  pixles.begin();
  pixels.clear();

  //RGB
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Blue, OUTPUT);

  //OLED
  display.begin(i2c_Address, true);
  display.clearDisplay();
  display.display();
  display.setTextColor(SH110x_WHITE, SH110X_BLACK);  //(0,1)

  //aht10
  if (!aht.begin()) {
    Serial.Write("Could Not Find AHT10 Sensor!");
    while (1) delay(10);
  }
}

void loop() {

  while (angle > 181 && angle < -1) {
    myServo.write(angle);
    delay(100);
    angle--;
    if(angle == 181 || angle == -1){
      if(angle == 181) angle = 0;
      if(angle == -1) angle = 180;
    }
    myServo.write(angle);
    delay(100);
  }
  
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

int getDirection(int currentAngle, int currentDir){ //function to check direction of servo and reset direcrion
  if (currentAngle >= 180) return -1; 
  if (currentAngle <= 0)   return 1;  
  return currentDir;                  
}
