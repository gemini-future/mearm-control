#include <Servo.h>
void chooseServoRun(char servoName, int angle, int delaytime);

Servo base;
Servo rArm;
Servo lArm;
Servo claw;

int initialangle_b=90;
int initialangle_r=90;
int initialangle_l=90;
int initialangle_c=90;

//测出各舵机的极限值(还未测量)(极限保护)
int baseMax;
int baseMin;
int rArmMax;
int rArmMin;
int lArmMax;
int lArmMin;
int clawMax;
int clawMin;

void setup() {
  base.attach(9);  //底座连9号引脚
  delay(200);
  lArm.attach(8);  //前臂连8号引脚
  delay(200);
  rArm.attach(7);  //后臂连7号引脚
  delay(200);
  claw.attach(6);  //爪子连6号引脚
  delay(200);

  base.write(90);
  delay(10);
  lArm.write(90);
  delay(10);
  rArm.write(90);
  delay(10);
  claw.write(90);
  delay(10);
  Serial.begin(9600);
  Serial.println("please input servoName and angle");
}

void loop() {
  int delaytime = 15;  //可变值

  if (Serial.available() > 0) {
    char servoName = Serial.read();
    Serial.print("servoName is");
    Serial.print(servoName);
    Serial.print("\n");
    int angle = Serial.parseInt();
    while (Serial.available() > 0) { Serial.read(); }
    if(servoName=='b' || servoName=='r' || servoName=='l' || servoName=='c'){
      chooseServoRun(servoName, angle, delaytime);}
    else if(servoName=='o'){
      Serial.print("initialServo_b value");
      Serial.println(base.read());
      Serial.print("initialServo_r value");
      Serial.println(rArm.read());
      Serial.print("initialServo_l value");
      Serial.println(lArm.read());
      Serial.print("initialServo_c value");
      Serial.println(claw.read());
    }
    else{
      Serial.println("your input is wrong");
    }
  }
}


//函数1
void chooseServoRun(char servoName, int angle, int delaytime) {
  int i;
  Servo myServo;
  switch (servoName) {

    case 'b':  //底座
      myServo = base;
      Serial.print("The base angle is");
      Serial.println(angle);
      break;

    case 'r':  //后臂
      myServo = rArm;
      Serial.print("The rArm angle is");
      Serial.println(angle);
      break;

    case 'l':  //前臂
      myServo = lArm;
      Serial.print("The lArm angle is");
      Serial.println(angle);
      break;

    case 'c':  //爪子
      myServo = claw;
      Serial.print("The claw angle is");
      Serial.println(angle);
      break;
  }
  int initialangle = myServo.read();
  if (initialangle < angle) {
    for (i = initialangle; i <= angle; i++) {
      myServo.write(i);
      delay(delaytime);
    }
  } else {
    for (i = initialangle; i >= angle; i--) {
      myServo.write(i);
      delay(delaytime);
    }
  }
}
