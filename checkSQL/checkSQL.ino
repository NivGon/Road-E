/*
  Road-E Project - Electronics
  
  Code To Check If Connectuon Between ESP32 And MySQL Is Possible With Python.
  The Code Contain The Next Sensors: AHT10, HSCR04, LDR.

  author: Ariel Gal
  date: 11-02-2026

  As Date 11-02:
  1. add library ArduinoJson - to handle data format in "clean" way 
  2. add settings for WIFI connection
  3. add declare for server URL - IP of the computer
  
*/

//libraries
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ESP32Servo.h>
#include <ArduinoJson.h>

//total defines
#define i2c_Address 0x3c

//hcsr04
#define Echo_Pin 33
#define Trig_Pin 32

//aht10
Adafruit_AHTX0 aht;

//LDR
#define LDR 34

//Network Settings
const char* ssid = "Modin-Students"; //WIFI Name 
const char* password = ""; //WIFI Password

//Server URL
const char* serverName = "http://COMPUTER_IP:5000/data"; //need to update any time before running + need to keep :5000/data at the end.


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

  //WIFI Check Connection
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WIFI");
  while(WiFi.status() != WL_CONNECTED){
    dealy(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WIFI network with IP Address: ");
  Serial.println(Wifi.LocalIP());
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

  //Check WIFI Connection Status
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;

    //Start Connection
    http.begin(serverName);
    http.addHeader("Contect-Type", "application/json");
  }



  
}

























