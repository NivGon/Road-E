/*
  Road-E Project - Electronics
  
  author: Ariel Gal
  date: 04-02-2026

  Changes In Code At Date 04-02:
  1. add libraries
  2. add function
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
#define Screen_Width 128 //OLED Display In WIdth (in pixels)
#define Screen_Height 64 //OLED Display In Height (in pixles)
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

  //aht10
  if(!aht.begin()){
    Serial.Write("Could Not Find AHT10 Sensor!");
    while(1) delay(10);
  }
}

void loop() {
  //aht10
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, temp);

  //calc distance hcsr04
  long duration; float distance;
  digitalWrite(Trig_Pin, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig_Pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig_Pin, LOW);
  duration = pulseIn(Echo_Pin, HIGH);
  distance = duration * 0.0343 / 2;

  //LDR
  int light = analogRead(LDR);
}

