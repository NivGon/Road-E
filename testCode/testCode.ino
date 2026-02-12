/*
  Road-E Project - Electronics

  author: Ariel Gal
  date: 12-02-2026

  Code File For Testing Sensors and Code Parts From The Main Code File

  Changes In Code At Date 12-02:
  1. change and check about possible working of the motors

*/

#define IR2 36  //Left Side - Connect to Pin 36
#define IR4 35  //Right Side - Connect to Pin 35

// --- Motor 1 Pins (Left Side) ---
#define Motor1_Pin1 12
#define Motor1_Pin2 13

// --- Motor 2 Pins (Right Side) ---
#define Motor2_Pin1 4
#define Motor2_Pin2 25

//Enable Pin
#define Motor_Enable 14

// --- PWM Settings ---
#define Frequency 30000
const int resolution = 8;    // Range between 0-255
const int motorSpeed = 160;  // Speed (0-255). 120 might be too slow for some motors, try 160.

// --- Logic Configuration ---
// Change these if your car reacts in reverse (e.g., stops on black instead of driving)
const int black = 1;
const int white = 0;

void setup() {
  Serial.begin(9600);

  pinMode(IR4, INPUT);
  pinMode(IR2, INPUT);

  // Connect Motors Direction Pins
//  pinMode(Motor1_Pin1, OUTPUT);
  pinMode(Motor1_Pin2, OUTPUT);
  pinMode(Motor2_Pin1, OUTPUT);
 // pinMode(Motor2_Pin2, OUTPUT);

  // Setup PWM for Speed Control
  // This uses ESP32 Core v3.0+ syntax
  if (!ledcAttach(Motor_Enable, Frequency, resolution)) {
    Serial.println("PWM Setup Failed!");
  }
}

void loop() {
  int leftReading = digitalRead(IR2);   // Pin 36
  int rightReading = digitalRead(IR4);  // Pin 35

  char direct = direction(leftReading, rightReading);

  switch (direct) {
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

  Serial.print("Left: ");
  Serial.print(leftReading);
  Serial.print(" | Right: ");
  Serial.println(rightReading);

  delay(50);  // Slow down so you can read it
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
  digitalWrite(Motor1_Pin1, HIGH);
  digitalWrite(Motor1_Pin2, LOW);

  // Motor 2 Forward
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
}

void turnRight() {
  // To turn Right: Keep Left motor moving, STOP Right motor
  ledcWrite(Motor_Enable, motorSpeed);

  // Left Motor Forward
  digitalWrite(Motor1_Pin1, HIGH);
 digitalWrite(Motor1_Pin2, LOW);

  // Right Motor Stop (Pivot turn)
  digitalWrite(Motor2_Pin1, LOW);
 digitalWrite(Motor2_Pin2, LOW);
}

void turnLeft() {
  // To turn Left: Keep Right motor moving, STOP Left motor
  ledcWrite(Motor_Enable, motorSpeed);

  // Left Motor Stop (Pivot turn)
  digitalWrite(Motor1_Pin1, LOW);
 digitalWrite(Motor1_Pin2, LOW);

  // Right Motor Forward
  digitalWrite(Motor2_Pin1, HIGH);
  digitalWrite(Motor2_Pin2, LOW);
}

void stopCar() {
  ledcWrite(Motor_Enable, 0); // Speed 0
  
  digitalWrite(Motor1_Pin1, LOW);
  digitalWrite(Motor1_Pin2, LOW);
  digitalWrite(Motor2_Pin1, LOW);
  digitalWrite(Motor2_Pin2, LOW);
}
