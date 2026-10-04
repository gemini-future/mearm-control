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
  int currentAngle;
  int goalAngle;
  unsigned long lastTime;
  int angleMin;
  int angleMax;
  bool isMoving;
};

//初始化四舵机
smoothServo arms[4] = {
  { {}, A0, 9, 90, 90, 0, baseMin, baseMax, false },  //左边x反向控制底座
  { {}, A1, 6, 90, 90, 0, clawMin, clawMax, false },  //左边y反向控制夹子(向上闭合，向下打开)
  { {}, A2, 8, 90, 90, 0, lArmMin, lArmMax, false },  //右边x反向控制前臂(左向上，右向下)
  { {}, A3, 7, 90, 90, 0, rArmMin, rArmMax, false },  //右边y反向控制后臂
};

//函数定义
void chooseServoRun(char instruction, int goalAngle);
void updateServo();
void run1();
void nowstate();
void joyStickControl(smoothServo &s);
void switchMode1();
void switchMode2();

void setup() {
  int i;
  for (i = 0; i < 4; i++) {
    arms[i].servo.attach(arms[i].pin);
    arms[i].servo.write(arms[i].currentAngle);
    delay(20);
  }
  Serial.begin(9600);
  Serial.println("please input instruction and goalAngle");
}

void loop() {
  ////////////输入b,r,l,c时要加数字////////////
  if (mode == 0) {
    updateServo();
    if (Serial.available() > 0) {
      char instruction = Serial.read();
      Serial.print("instruction is");
      Serial.print(instruction);
      Serial.print("\n");
      int goalAngle = Serial.parseInt();
      while (Serial.available() > 0) { Serial.read(); }
      if (instruction == 'b' || instruction == 'r' || instruction == 'l' || instruction == 'c') {
        chooseServoRun(instruction, goalAngle);
      } else if (instruction == 'k') {  //按k查看现在的状态
        nowstate();
      } else if (instruction == 'i') {  //初始化
        run1();
      } else if (instruction == 'O') {  //打开夹子
        chooseServoRun('c', clawMin);
      } else if (instruction == 'S') {  //关闭夹子
        chooseServoRun('c', clawMax);
      } else if (instruction == 'H') {  //加速
        interval -= 5;
        if (interval < 2) {
          interval = 2;
          Serial.println("Warning:achieve minimum");
        }
      } else if (instruction == 'L') {  //减速
        interval += 5;
        if (interval > 50) {
          interval = 50;
          Serial.println("Warning:achieve maximum");
        }
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
void chooseServoRun(char instruction, int goalAngle) {
  int idx;
  switch (instruction) {

    case 'b':  //底座
      idx = 0;
      Serial.print("The base angle is");
      break;

    case 'r':  //后臂
      idx = 3;
      Serial.print("The rArm angle is");
      break;

    case 'l':  //前臂
      idx = 2;
      Serial.print("The lArm angle is");
      break;

    case 'c':  //爪子
      idx = 1;
      Serial.print("The claw angle is");
      break;

    default:
      Serial.println("your input is wrong");
      return;
  }
  if (goalAngle > arms[idx].angleMax) { goalAngle = arms[idx].angleMax; }
  if (goalAngle < arms[idx].angleMin) { goalAngle = arms[idx].angleMin; }

  arms[idx].goalAngle = goalAngle;
  arms[idx].isMoving = true;
  Serial.println(arms[idx].goalAngle);
}

//函数2
void updateServo() {
  int i;
  currentTime = millis();
  for (i = 0; i < 4; i++) {
    if (!arms[i].isMoving) { continue; }

    if (currentTime - arms[i].lastTime > interval) {
      if (arms[i].currentAngle > arms[i].goalAngle) {
        arms[i].currentAngle--;
      } else if (arms[i].currentAngle < arms[i].goalAngle) {
        arms[i].currentAngle++;
      } else {
        arms[i].isMoving = false;
      }
      arms[i].servo.write(arms[i].currentAngle);
      arms[i].lastTime = currentTime;
    }
  }
}

//函数3
void run1() {  //初始值要改
  int i;
  int action1[4][2] = {
    { 'b', 90 },
    { 'r', 90 },
    { 'l', 90 },
    { 'c', 90 },
  };
  for (i = 0; i < 4; i++) {
    chooseServoRun(action1[i][0], action1[i][1]);
  }
}

//函数4
void nowstate() {
  Serial.print("nowServo_b value ");
  Serial.println(arms[0].currentAngle);
  Serial.print("nowServo_r value ");
  Serial.println(arms[3].currentAngle);
  Serial.print("nowServo_l value ");
  Serial.println(arms[2].currentAngle);
  Serial.print("nowServo_c value ");
  Serial.println(arms[1].currentAngle);
  Serial.print("dealytime value ");
  Serial.println(interval);
}

//函数5
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
    s.currentAngle += step;
    if (s.currentAngle > s.angleMax) { s.currentAngle = s.angleMax; }
    if (s.currentAngle < s.angleMin) { s.currentAngle = s.angleMin; }
    s.servo.write(s.currentAngle);
  }
  s.lastTime = currentTime;
}

//函数6
void switchMode1() {
  mode = 1;
  Serial.println("--------Joystick mode is activated--------");
}

//函数7
void switchMode2() {
  mode = 0;
  Serial.println("--------instruction mode is activated--------");
}
