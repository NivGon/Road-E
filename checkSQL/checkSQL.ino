/*
  Road-E Project - Electronics
  
  Code To Check If Connectuon Between ESP32 And MySQL Is Possible With Python.
  The Code Contain The Next Sensors: AHT10, HSCR04, LDR.

  author: Ariel Gal
  date: 06-02-2026

  As Date 06-02:
  1. add start of code
  
*/

//libraries
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ESP32Servo.h>

//total defines
#define i2c_Address 0x3c

//hcsr04
#define Echo_Pin 33
#define Trig_Pin 32

//aht10
Adafruit_AHTX0 aht;

//LDR
#define LDR 34

void setup(){
  Serial.begin(9600);

  //hcsr
  pinMode(Echo_Pin, INPUT);
  pinMode(Trig_Pin, OUTPUT);

  //aht10
  if (!aht.begin()) {
    Serial.println("Could Not Find AHT10 Sensor!");
    while (1) delay(10);
  }

  //LDR
  pinMode(LDR, INPUT);

  //Servo
  myServo.attach(Servo_Pin);
  myServo.write(0);
  delay(500);
  myServo.write(180);
  delay(500);
  myServo.write(90);
}


void loop(){
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  delay(100);

  int light = analogRead(LDR);

  dealy(100);

  //calc distance hcsr04
  long duration;
  float distance;
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = duration * 0.034 / 2;
  Serial.print("distance -> ");
  Serial.println(distance);

  
}

























