#include <Servo.h>
Servo myServo;

void setup() {
  myServo.attach(9); //舵机接9号引脚
}

void loop() {
  myServo.write(0);
  delay(1000);
  myServo.write(90);
  delay(1000);
  myServo.write(180);
  delay(1000);
}