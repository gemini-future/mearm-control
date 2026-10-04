#include <Servo.h>

//测出各舵机的极限值(还未测量)(极限保护)
const int baseMax = 180;
const int baseMin = 0;
const int clawMax = 180;
const int clawMin = 0;
const int lArmMax = 180;
const int lArmMin = 0;
const int rArmMax = 180;
const int rArmMin = 0;

int mode = 1;  //1为摇杆模式//0为指令模式

unsigned long currentTime = 0;
unsigned long interval = 15;
int dead = 20;  //死区范围

struct smoothServo {
  Servo servo;
  int joyPin;
  int pin;
  int initialAngle;
  int goalAngle;
  unsigned long lastTime;
  int angleMin;
  int angleMax;
};

//初始化四舵机
smoothServo arms[4] = {
  { {}, A0, 9, 90, 90, 0, baseMin, baseMax },  //左边x反向控制底座
  { {}, A1, 6, 90, 90, 0, clawMin, clawMax },  //左边y反向控制夹子
  { {}, A2, 8, 90, 90, 0, lArmMin, lArmMax },  //右边x反向控制前臂(左向上，右向下)
  { {}, A3, 7, 90, 90, 0, rArmMin, rArmMax },  //右边y反向控制后臂
};

void chooseServoRun(char instruction, int goalAngle, int delaytime);
void run1(int delaytime);
void nowstate();
void switchMode1();
void switchMode2();
void joyStickControl(smoothServo &s);

void setup() {
  int i;
  for (i = 0; i < 4; i++) {
    arms[i].servo.attach(arms[i].pin);
    arms[i].servo.write(arms[i].initialAngle);
    delay(20);
  }
  Serial.begin(9600);
  Serial.println("please input instruction and goalAngle");
}

void loop() {
  if (mode == 0) {       ////指令模式////
    int delaytime = 15;  //可变值
    ////////////输入b,r,l,c时要加数字////////////
    if (Serial.available() > 0) {
      char instruction = Serial.read();
      Serial.print("instruction is");
      Serial.print(instruction);
      Serial.print("\n");

      int goalAngle = Serial.parseInt();
      while (Serial.available() > 0) { Serial.read(); }
      if (instruction == 'b' || instruction == 'r' || instruction == 'l' || instruction == 'c') {
        chooseServoRun(instruction, goalAngle, delaytime);
      } else if (instruction == 'o') {
        nowstate();
      } else if (instruction == 'i') {
        run1(delaytime);
      } else if (instruction == 'm') {
        switchMode1();  //按m切换为摇杆模式
      }
    }
  } else {
    if (Serial.available() > 0) {
      char instruction = Serial.read();
      Serial.print("instruction is");
      Serial.print(instruction);
      Serial.print("\n");
      if (instruction == 'n') {
        switchMode2();  //按n切换为指令模式
      }
    }
  }

  if (mode == 1) {  ////摇杆模式////
    int i;
    for (i = 0; i < 4; i++) {
      joyStickControl(arms[i]);
    }
  }
}



//函数1
void chooseServoRun(char instruction, int goalAngle, int delaytime) {
  int i;
  int idx;
  Servo *myservo;
  switch (instruction) {

    case 'b':  //底座
      if (goalAngle > baseMax) { goalAngle = baseMax; }
      if (goalAngle < baseMin) { goalAngle = baseMin; }
      idx = 0;
      Serial.print("The base angle is");
      Serial.println(goalAngle);
      break;

    case 'r':  //后臂
      if (goalAngle > rArmMax) { goalAngle = rArmMax; }
      if (goalAngle < rArmMin) { goalAngle = rArmMin; }
      idx = 3;
      Serial.print("The rArm angle is");
      Serial.println(goalAngle);
      break;

    case 'l':  //前臂
      if (goalAngle > lArmMax) { goalAngle = lArmMax; }
      if (goalAngle < lArmMin) { goalAngle = lArmMin; }
      idx = 2;
      Serial.print("The lArm angle is");
      Serial.println(goalAngle);
      break;

    case 'c':  //爪子
      if (goalAngle > clawMax) { goalAngle = clawMax; }
      if (goalAngle < clawMin) { goalAngle = clawMin; }
      idx = 1;
      Serial.print("The claw angle is");
      Serial.println(goalAngle);
      break;

    default:
      Serial.println("your input is wrong");
      return;
  }
  myservo = &arms[idx].servo;
  if (arms[idx].initialAngle < goalAngle) {
    for (i = arms[idx].initialAngle; i <= goalAngle; i++) {
      myservo->write(i);
      delay(delaytime);
    }
  } else {
    for (i = arms[idx].initialAngle; i >= goalAngle; i--) {
      myservo->write(i);
      delay(delaytime);
    }
  }
  arms[idx].initialAngle = goalAngle;
}

//函数2
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

//函数3
void nowstate() {
  Serial.print("nowServo_b value ");
  Serial.println(arms[0].servo.read());
  Serial.print("nowServo_r value ");
  Serial.println(arms[3].servo.read());
  Serial.print("nowServo_l value ");
  Serial.println(arms[2].servo.read());
  Serial.print("nowServo_c value ");
  Serial.println(arms[1].servo.read());
}

//函数4
void switchMode1() {
  mode = 1;
  Serial.println("--------Joystick mode is activated--------");
}
//函数5
void switchMode2() {
  mode = 0;
  Serial.println("--------instruction mode is activated--------");
}

//函数6
void joyStickControl(smoothServo &s) {
  int step = 0;
  currentTime = millis();
  if (currentTime - s.lastTime < interval) {
    return;
  }
  int raw = analogRead(s.joyPin);
  if (raw > 512 + dead) {
    step = map(raw, 512 + dead, 1023, 1, 5);
  } else if (raw < 512 - dead) {
    step = map(raw, 0, 512 - dead, -5, -1);
  }
  if (step != 0) {
    s.initialAngle += step;
    if (s.initialAngle > s.angleMax) { s.initialAngle = s.angleMax; }
    if (s.initialAngle < s.angleMin) { s.initialAngle = s.angleMin; }
    s.servo.write(s.initialAngle);
  }
  s.lastTime = currentTime;
}
