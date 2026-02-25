/*
  Road-E Project - Electronics
  
  Code To Check If Connectuon Between ESP32 And MySQL Is Possible With Python.
  The Code Contain The Next Sensors: AHT10, HSCR04, LDR.

  author: Ariel Gal
  date: 16-02-2026

  As Date 16-02:

  
  How To Run This Code?
  1. Upload this code to the esp32 (Attention to check that the info of the WIFI & IP are correct).
  2. Run python code named "server.py" via cmd (Use cd to get into the folder, and than write "python server.py").
  3. The Python code runs as server on the cmd, you don't need to run it via IDE.
  4. Once the Python is running, open the SQL and enter "SELECT * FROM sensor_reading" and you can see the data in the table.
  5. Repeat on the same as 4 every few minutes to see updates.


*/

//libraries
#include <Wire.h>
#include <WiFi.h>
#include <HTTPClient.h>
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
const char* ssid = "WIFI_NAME";  //WIFI Name
const char* password = "WIFI_PASSWORD";     //WIFI Password

//Server URL
const char* serverName = "http://COMPUTER_IP:5000/upload";  //need to update any time before running + need to keep :5000/data at the end.


void setup() {
  Serial.begin(115200);

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

  //WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}


void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    sensors_event_t humidity, temp;
    aht.getEvent(&humidity, &temp);
    float t = temp.temperature;
    float h = humidity.relative_humidity;
    delay(100);

    int light = analogRead(LDR);
    delay(100);

    //calc distance hcsr04
    long duration;
    float distance;
    digitalWrite(Trig_Pin, LOW);
    delayMicroseconds(2);
    digitalWrite(Trig_Pin, HIGH);
    delayMicroseconds(10);
    digitalWrite(Trig_Pin, LOW);
    duration = pulseIn(Echo_Pin, HIGH);
    distance = duration * 0.034 / 2;
    Serial.print("distance -> ");
    Serial.println(distance);

    // --- DEBUG PRINT ---
    Serial.printf("Temp: %.2f, Hum: %.2f, Dist: %.2f, LDR: %d\n", t, h, distance, light);

    // --- SEND TO PYTHON ---
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");

    // Construct JSON String
    String jsonPayload = "{";
    jsonPayload += "\"temp\":" + String(t) + ",";
    jsonPayload += "\"hum\":" + String(h) + ",";
    jsonPayload += "\"dist\":" + String(distance) + ",";
    jsonPayload += "\"ldr\":" + String(light);
    jsonPayload += "}";

    int httpResponseCode = http.POST(jsonPayload);

    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.println("Server Response: " + response);
    } else {
      Serial.print("Error sending POST: ");
      Serial.println(httpResponseCode);
    }
    http.end();

  } else {
    Serial.println("WiFi Disconnected");
  }

  delay(5000);  // Send data every 5 seconds
}
