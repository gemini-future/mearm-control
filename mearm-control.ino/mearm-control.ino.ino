#include <Servo.h>
void chooseServoRun(char servoName, int angle, int delaytime);
void run1(int delaytime);
void run2(int delaytime);

Servo base;
Servo rArm;
Servo lArm;
Servo claw;

const int initialangle_b = 90;
const int initialangle_r = 90;
const int initialangle_l = 90;
const int initialangle_c = 90;

//测出各舵机的极限值(还未测量)(极限保护)
const int baseMax;
const int baseMin;
const int rArmMax;
const int rArmMin;
const int lArmMax;
const int lArmMin;
const int clawMax;
const int clawMin;

void setup() {
  base.attach(9);  //底座连9号引脚
  delay(200);
  lArm.attach(8);  //前臂连8号引脚
  delay(200);
  rArm.attach(7);  //后臂连7号引脚
  delay(200);
  claw.attach(6);  //爪子连6号引脚
  delay(200);

  base.write(initialangle_b);
  delay(10);
  lArm.write(initialangle_l);
  delay(10);
  rArm.write(initialangle_r);
  delay(10);
  claw.write(initialangle_c);
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
    if (servoName == 'b' || servoName == 'r' || servoName == 'l' || servoName == 'c') {
      chooseServoRun(servoName, angle, delaytime);
    } else if (servoName == 'o') {
      Serial.print("nowServo_b value");
      Serial.println(base.read());
      Serial.print("nowServo_r value");
      Serial.println(rArm.read());
      Serial.print("nowServo_l value");
      Serial.println(lArm.read());
      Serial.print("nowServo_c value");
      Serial.println(claw.read());
    } else if (servoName == 'i') {
      run1(delaytime);
    } else if(servoName=='p'){
      run2(delaytime);
    } else {
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

void run1(int delaytime) {
  int i;
  int action1[4][3] = {
    { 'b', 90, delaytime },
    { 'r', 90, delaytime },
    { 'l', 90, delaytime },
    { 'c', 90, delaytime },
  };
  for (i = 0; i < 4; i++) {
    chooseServoRun(action1[i][0], action1[i][1], action1[i][2]);
  }
}

void run2(int delaytime) {
  int i;
  int complexAction[8][3] = {
    { 'b', 77, delaytime },
    { 'r', 99, delaytime },
    { 'l', 150, delaytime },
    { 'c', 68, delaytime },
    { 'r', 23, delaytime },
    { 'l', 44, delaytime },
    { 'b', 164, delaytime },
    { 'c', 111, delaytime },
  };
  for (i = 0; i < 8; i++) {
    chooseServoRun(complexAction[i][0], complexAction[i][1], complexAction[i][2]);
  }
}








