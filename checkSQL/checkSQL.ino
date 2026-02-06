#include <ESP32Servo.h>
Servo myServo;
#define Servo_Pin 18 // Changed to 13 to avoid ESP32-CAM SD card conflicts

int angle = 180;
int direction = -1; // Global variable to remember the way we are moving

void setup() {
  myServo.attach(Servo_Pin);
  myServo.write(angle);
  delay(500);
}

void loop() {
  myServo.write(angle);

  // Update the direction based on the current angle
  direction = getDirection(angle, direction);

  switch (direction) {
    case 1:
      angle++;
      break;
    case -1:
      angle--;
      break;
  }
  
  delay(30); 
}

// Function to decide if we should flip direction
int getDirection(int currentAngle, int currentDir) {
  if (currentAngle >= 180) return -1; // Hit the top, go down
  if (currentAngle <= 0)   return 1;  // Hit the bottom, go up
  return currentDir;                  // Otherwise, keep going the same way
}