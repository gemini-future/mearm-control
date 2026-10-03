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
  { {}, A0, 9, 90, 90, 0  },  //左边x反向控制底座
  { {}, A1, 6, 90, 90, 0, },  //左边y反向控制夹子
  { {}, A2, 8, 90, 90, 0, },  //右边x反向控制前臂(左向上，右向下)
  { {}, A3, 7, 90, 90, 0, },  //右边y反向控制后臂
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
  ////////////输入b,r,l,c时要加数字////////////
  if (Serial.available() > 0) {
    char instruction = Serial.read();
    Serial.print("instruction is");
    Serial.print(instruction);
    Serial.print("\n");
    if (mode == 0) {       ////指令模式////
      int delaytime = 15;  //可变值
      int goalAngle = Serial.parseInt();
      while (Serial.available() > 0) { Serial.read(); }
      if (instruction == 'b' || instruction == 'r' || instruction == 'l' || instruction == 'c') {
        int x;
        for(x=0;x<4;x++){
        chooseServoRun(instruction, goalAngle, delaytime);}
      } else if (instruction == 'o') {
        nowstate();
      } else if (instruction == 'i') {
        run1(delaytime);
      } else if (instruction == 'm') {
        switchMode1();  //按m切换为摇杆模式
      } else {
        Serial.println("your input is wrong");
      }
    } else {
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
  Servo myServo;
  switch (instruction) {

    case 'b':  //底座
    if(goalAngle>baseMax){goalAngle=baseMax;}
    if(goalAngle<baseMin){goalAngle=baseMin;}
      myServo = arms[0].servo;
      Serial.print("The base angle is");
      Serial.println(goalAngle);
      break;

    case 'r':  //后臂
    if(goalAngle>rArmMax){goalAngle=rArmMax;}
    if(goalAngle<rArmMin){goalAngle=rArmMin;}
      myServo = arms[3].servo;
      Serial.print("The rArm angle is");
      Serial.println(goalAngle);
      break;

    case 'l':  //前臂
    if(goalAngle>lArmMax){goalAngle=lArmMax;}
    if(goalAngle<lArmMin){goalAngle=lArmMin;}
      myServo = arms[2].servo;
      Serial.print("The lArm angle is");
      Serial.println(goalAngle);
      break;

    case 'c':  //爪子
    if(goalAngle>clawMax){goalAngle=clawMax;}
    if(goalAngle<clawMin){goalAngle=clawMin;}
      myServo = arms[1].servo;
      Serial.print("The claw angle is");
      Serial.println(goalAngle);
      break;
  }
  int initialangle = myServo.read();
  if (initialangle < goalAngle) {
    for (i = initialangle; i <= goalAngle; i++) {
      myServo.write(i);
      delay(delaytime);
    }
  } else {
    for (i = initialangle; i >= goalAngle; i--) {
      myServo.write(i);
      delay(delaytime);
    }
  }
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
  while (Serial.available() > 0) { Serial.read(); }
}

//函数6
void joyStickControl(smoothServo &s) {
  currentTime = millis();
  if (currentTime - s.lastTime < interval) {
    return;
  } else {
    int raw = analogRead(s.joyPin);
    if (raw > 512 + dead || raw < 512 - dead) {  //防止机械臂抖动
      s.lastTime = currentTime;
      s.initialAngle = s.servo.read();
      s.goalAngle = map(raw, 0, 1023, 0, 180);
      if (s.initialAngle > s.goalAngle) {
        s.initialAngle -= 1;
      } else {
        s.initialAngle += 1;
      }
      s.servo.write(s.initialAngle);
    }
  }
}
