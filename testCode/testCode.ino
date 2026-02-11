/*
  Road-E Project - Electronics

  author: Ariel Gal
  date: 11-02-2026

  Code File For Testing Sensors and Code Parts From The Main Code File

  Changes In Code At Date 11-02:
  1. Check Engines And Wheels
  2. Code For The Car To Drive In Circle

*/

//Motor 1 Pins - Left Side
#define Motor1_Pin1 12
#define Motor1_Pin2 13

//Motor 2 Pins - Right Side
#define Motor2_Pin1 25
#define Motor2_Pin2 4

#define Motor_Enable 14

//PWM Settings
#define Frequancy 30000
const int resolution = 8;    //Range between 0-255
const int motorSpeed = 120;  //High speed to ensure movement


void setup() {
  Serial.begin(9600);

  //Connect Motors
  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
  pinMode(Motor2_Pin2, OUTPUT);

  // Attach PWM
  ledcAttach(Motor_Enable, Frequancy, resolution);
  ledcAttach(Motor_Enable, Frequancy, resolution);
}

void loop() {

  for (int i = 0; i < 4; i++) {
    //Set direction of car forword
    Serial.println("Moving Forward");
    digitalWrite(Motor1_Pin1, LOW);
    digitalWrite(Motor1_Pin2, HIGH);
    digitalWrite(Motor2_Pin1, HIGH);
    digitalWrite(Motor2_Pin2, LOW);
    ledcWrite(Motor_Enable, motorSpeed);
    delay(2000);

    //Set direction of car left
    Serial.println("Moving Left");
    digitalWrite(Motor1_Pin1, HIGH);
    digitalWrite(Motor1_Pin2, LOW);
    digitalWrite(Motor2_Pin1, LOW);
    digitalWrite(Motor2_Pin2, HIGH);
    ledcWrite(Motor_Enable, motorSpeed);
    delay(1000);
  }

  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, HIGH);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, HIGH);
  ledcWrite(Motor_Enable, motorSpeed);
  delay(1500);
}
