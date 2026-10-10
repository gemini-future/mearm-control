#include <Servo.h>
#include <math.h>
#include <SoftwareSerial.h>

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

//task2
bool task2Running = false;
int task2Step = 0;
const int task2TotalStep = 8;
char group = 'A';
unsigned char (*currentAction)[8][4][2];

const unsigned char actionA[8][4][2] PROGMEM = {
  //具体数值未测量
  { { 'b', 1 }, { 'l', 2 }, { 'r', 3 }, { 'c', 4 } },
  { { 'b', 179 }, { 'l', 178 }, { 'r', 177 }, { 'c', 176 } },
  { { 'b', 1 }, { 'l', 2 }, { 'r', 3 }, { 'c', 4 } },
  { { 'b', 179 }, { 'l', 178 }, { 'r', 177 }, { 'c', 176 } },
  { { 'b', 1 }, { 'l', 2 }, { 'r', 3 }, { 'c', 4 } },
  { { 'b', 179 }, { 'l', 178 }, { 'r', 177 }, { 'c', 176 } },
  { { 'b', 1 }, { 'l', 2 }, { 'r', 3 }, { 'c', 4 } },
  { { 'b', 179 }, { 'l', 178 }, { 'r', 177 }, { 'c', 176 } },
};
const unsigned char actionB[8][4][2] PROGMEM = {
  //具体数值未测量
  { { 'b', 45 }, { 'l', 135 }, { 'r', 45 }, { 'c', 135 } },
  { { 'b', 135 }, { 'l', 45 }, { 'r', 135 }, { 'c', 45 } },
  { { 'b', 45 }, { 'l', 135 }, { 'r', 45 }, { 'c', 135 } },
  { { 'b', 135 }, { 'l', 45 }, { 'r', 135 }, { 'c', 45 } },
  { { 'b', 45 }, { 'l', 135 }, { 'r', 45 }, { 'c', 135 } },
  { { 'b', 135 }, { 'l', 45 }, { 'r', 135 }, { 'c', 45 } },
  { { 'b', 45 }, { 'l', 135 }, { 'r', 45 }, { 'c', 135 } },
  { { 'b', 135 }, { 'l', 45 }, { 'r', 135 }, { 'c', 45 } },
};
const unsigned char actionC[8][4][2] PROGMEM = {
  //具体数值未测量
  { { 'b', 1 }, { 'l', 2 }, { 'r', 3 }, { 'c', 4 } },
  { { 'b', 179 }, { 'l', 178 }, { 'r', 177 }, { 'c', 176 } },
  { { 'b', 45 }, { 'l', 46 }, { 'r', 47 }, { 'c', 48 } },
  { { 'b', 135 }, { 'l', 134 }, { 'r', 133 }, { 'c', 132 } },
  { { 'b', 1 }, { 'l', 2 }, { 'r', 3 }, { 'c', 4 } },
  { { 'b', 179 }, { 'l', 178 }, { 'r', 177 }, { 'c', 176 } },
  { { 'b', 45 }, { 'l', 44 }, { 'r', 43 }, { 'c', 42 } },
  { { 'b', 135 }, { 'l', 136 }, { 'r', 137 }, { 'c', 138 } },
};

unsigned long currentTime = 0;
unsigned long interval = 15;
int dead = 50;  //死区范围

//录制
bool isRecording = false;
int recordIndex = 0;
unsigned char recordData[150][4];
unsigned long lastRecordTime = 0;

//重播
bool isPlaying = false;
unsigned long lastPlayTime = 0;
int playIndex = 0;

//机械结构参数（未测）
const float L1 = 80.0;  //大臂长度
const float L2 = 80.0;  //小臂长度
const float d = 80.0;   //笔到夹子的距离
//零点偏移（未测）
const float baseZero = 90.0;
const float rArmZero = 90.0;
const float lArmZero = 90.0;
//方向修正(当舵机方向相反时改为-1.0)
const float baseDir = 1.0;
const float lArmDir = 1.0;
const float rArmDir = 1.0;
//抬笔，落笔的距离
int PEN_DOWN = 0;  //落笔：接触纸面（拿笔时笔尖最好接触纸面--保证d不变）
int PEN_UP = 30;   //抬笔：离开纸面
const float pi = 3.1415926;

unsigned long lastDrawTime = 0;
const unsigned char drawInterval = 80;  //可改
float pathX[10];                        //航电x坐标
float pathY[10];                        //航电y坐标
int pathCount = 0;                      //航点数量
int currentSegment = 0;                 //现在是第几部分
int segmentStep = 0;                    //某一段中的第几步
int segmentStepCount = 25;              //一段总共有多少步

enum DrawState {
  DRAW_IDLE,     //空闲
  DRAW_RUNNING,  //运行
  DRAW_PAUSED    //暂停
};
DrawState drawState = DRAW_IDLE;

enum DrawStep {
  STEPONE_LIFT,    //移动到起点上方
  STEPTWO_LOWER,   //落笔
  STEPTHREE_DRAW,  //绘画
  STEPFOUR_FINISH  //结束
};
DrawStep drawStep = STEPFOUR_FINISH;


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
  { {}, A1, 6, 90, 90, 0, clawMin, clawMax, false },  //左边y反向控制夹子(向下闭合，向上打开)
  { {}, A2, 8, 90, 90, 0, lArmMin, lArmMax, false },  //右边x反向控制前臂(左向上，右向下)
  { {}, A3, 7, 90, 90, 0, rArmMin, rArmMax, false },  //右边y反向控制后臂
};

//函数定义
void chooseServoRun(char instruction, int goalAngle);
void updateServo();
void goHome();
void nowstate();
void joyStickControl(smoothServo &s);
void switchMode1();
void switchMode2();
void task2();
void run2Start();
void run3Start();
void run4Start();
void parseCmd(char instruction, bool fromSerial);
void Record();
void playRecordedAction();
void updatePlay();
//======================================================初始化=====================================================================
void setup() {
  int i;
  for (i = 0; i < 4; i++) {
    arms[i].servo.attach(arms[i].pin);
    arms[i].servo.write(arms[i].currentAngle);
    delay(20);
  }
  Serial.setTimeout(50);
  Serial.begin(9600);
  Serial.println("please input instruction and goalAngle");
}
//=======================================================主循环=========================================================================
void loop() {
  if (isRecording) { Record(); }
  updateDraw();
  ////////////输入b,r,l,c时要加数字////////////
  if (mode == 0) {
    updateServo();
    task2();
    if (isPlaying) { updatePlay(); }
    while (Serial.available() > 0) {
      char instruction = Serial.read();
      if (instruction == ' ' || instruction == '\n' || instruction == '\r') { continue; }
      parseCmd(instruction, false);
    }

  } else {
    while (Serial.available() > 0) {
      char instruction = Serial.read();
      if (instruction == ' ' || instruction == '\n' || instruction == '\r') { continue; }
      Serial.print("instruction is");
      Serial.print(instruction);
      Serial.print("\n");
      if (instruction == 'n') {
        switchMode2();                  //按n切换为指令模式
      } else if (instruction == 'W') {  //结束录制
        isRecording = false;
        mode = 0;
        Serial.println("Record end ! Joystick mode turn off !");
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

//===============================================函数======================================================================
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
  arms[idx].lastTime = millis();
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
void goHome() {
  chooseServoRun('b', 90);
  chooseServoRun('c', 90);
  chooseServoRun('l', 90);
  chooseServoRun('r', 90);
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
  int i;
  for (i = 0; i < 4; i++) {
    arms[i].isMoving = false;
  }
}

//函数7
void switchMode2() {
  mode = 0;
  Serial.println("--------instruction mode is activated--------");
  int i;
  for (i = 0; i < 4; i++) {
    arms[i].currentAngle = arms[i].servo.read();
  }
}

//函数8
void task2() {
  int i;

  if (group == 'A') {
    currentAction = &actionA;
  } else if (group == 'B') {
    currentAction = &actionB;
  } else if (group == 'C') {
    currentAction = &actionC;
  }
  bool stepAllDone = false;
  if (arms[0].isMoving == false && arms[1].isMoving == false && arms[2].isMoving == false && arms[3].isMoving == false) { stepAllDone = true; }
  if (!task2Running) { return; }
  if (task2Step >= task2TotalStep) {
    task2Running = false;
    Serial.println("task2 finished");
    return;
  }
  if (stepAllDone) {
    Serial.print("task2Step is ");
    Serial.println(task2Step);
    for (i = 0; i < 4; i++) {
      char cmd = pgm_read_byte(&(*currentAction)[task2Step][i][0]);
      int ang = pgm_read_byte(&(*currentAction)[task2Step][i][1]);
      chooseServoRun(cmd, ang);
    }
    task2Step++;
  }
}

//函数9
void run2Start() {
  task2Running = true;
  task2Step = 0;
  Serial.println("run2 start");
  group = 'A';
}

//函数10
void run3Start() {
  task2Running = true;
  task2Step = 0;
  Serial.println("run3 start");
  group = 'B';
}

//函数11
void run4Start() {
  task2Running = true;
  task2Step = 0;
  Serial.println("run4 start");
  group = 'C';
}

//函数12
void parseCmd(char instruction, bool fromSerial) {
  if (instruction == 'b' || instruction == 'r' || instruction == 'l' || instruction == 'c') {
    int goalAngle;
    if (fromSerial == false) {
      goalAngle = Serial.parseInt();
    } else if (fromSerial == true) {
    }
    chooseServoRun(instruction, goalAngle);
  } else if (instruction == 'k') {  //按k查看现在的状态
    nowstate();
  } else if (instruction == 'O') {  //打开夹子
    chooseServoRun('c', clawMin);
  } else if (instruction == 'S') {  //关闭夹子
    chooseServoRun('c', clawMax);
  } else if (instruction == 'H') {  //加速
    interval -= 5;
    if (interval < 2) {
      interval = 2;
      Serial.println("Warning:delayTime achieve minimum");
    }
  } else if (instruction == 'L') {  //减速
    interval += 5;
    if (interval > 50) {
      interval = 50;
      Serial.println("Warning:delaytime achieve maximum");
    }
  } else if (instruction == 'm') {
    switchMode1();  //按m切换为摇杆模式
  } else if (instruction == 'A') {
    run2Start();
  } else if (instruction == 'B') {
    run3Start();
  } else if (instruction == 'C') {
    run4Start();
  } else if (instruction == 'Q') {  //开始录制
    isRecording = true;
    recordIndex = 0;
    lastRecordTime = 0;
    mode = 1;
    Serial.println("Record start ! Joystick mode turn on !");
  } else if (instruction == 'P') {  //重播
    Serial.println("Play start !");
    playRecordedAction();
  } else if (instruction == 'E') {  //回中
    goHome();
  } else if (instruction == 'J') {  //直接画直线
    makeline();
  }
}

//函数13
void Record() {
  if (recordIndex >= 150) {
    isRecording = false;
    Serial.println("Record full !");
    mode=0;
    Serial.println("instruction mode turn on !");
    return;
  }

  if (millis() - lastRecordTime > 100) {
    lastRecordTime = millis();
    recordData[recordIndex][0] = arms[0].currentAngle;  //底座
    recordData[recordIndex][1] = arms[1].currentAngle;  //夹子
    recordData[recordIndex][2] = arms[2].currentAngle;  //前臂
    recordData[recordIndex][3] = arms[3].currentAngle;  //后臂
    recordIndex++;
  }
}


//函数14
void playRecordedAction() {
  isPlaying = true;
  lastPlayTime = 0;
  playIndex = 0;
}

//函数15
void updatePlay() {
  if (!isPlaying) { return; }
  if (playIndex >= recordIndex) { isPlaying = false; }
  if (millis() - lastPlayTime >= 100) {
    lastPlayTime = millis();
    chooseServoRun('b', recordData[playIndex][0]);
    chooseServoRun('c', recordData[playIndex][1]);
    chooseServoRun('l', recordData[playIndex][2]);
    chooseServoRun('r', recordData[playIndex][3]);
    playIndex++;
  }
}

//逆运动学
bool solveIK(float x, float y, float z, int &bAngle, int &lAngle, int &rAngle) {
  float theta1 = atan2(y, x);
  float r = sqrt(x * x + y * y);
  float k = z + d;
  float L = sqrt(r * r + k * k);
  float costheta3 = (L * L - L1 * L1 - L2 * L2) / (2 * L1 * L2);
  if (costheta3 > 1.0 || costheta3 < -1.0) { return false; }
  float theta3 = acos((L * L - L1 * L1 - L2 * L2) / (2 * L1 * L2));
  float theta2 = atan2(k, r) - atan2(L2 * sin(theta3), L1 + L2 * cos(theta3));
  bAngle = (int)(theta1 * 180.0 / pi) * baseDir + baseZero;
  lAngle = (int)(theta3 * 180.0 / pi) * lArmDir + lArmZero;
  rAngle = (int)(theta2 * 180.0 / pi) * rArmDir + rArmZero;
  bAngle = constrain(bAngle, baseMin, baseMax);
  lAngle = constrain(lAngle, lArmMin, lArmMax);
  rAngle = constrain(rAngle, rArmMin, rArmMax);
  return true;
}

void moveTo(float x, float y, float z) {
  int bAngle;
  int lAngle;
  int rAngle;
  if (solveIK(x, y, z, bAngle, lAngle, rAngle)) {
    chooseServoRun('b', bAngle);
    chooseServoRun('l', lAngle);
    chooseServoRun('r', rAngle);
  }
}

void startDraw() {
  if (pathCount < 2) {
    Serial.println("no path !");
    return;
  }
  drawState = DRAW_RUNNING;
  drawStep = STEPONE_LIFT;
  currentSegment = 0;
  segmentStep = 0;
  lastDrawTime = 0;
  Serial.println("draw start !");
}

void updateDraw() {
  if (drawState != DRAW_RUNNING) { return; }
  bool allRunDone;
  if (arms[0].isMoving == false && arms[2].isMoving == false && arms[3].isMoving == false) { allRunDone = true; }
  switch (drawStep) {

    case STEPONE_LIFT:
      moveTo(pathX[0], pathY[0], PEN_UP);
      drawStep = STEPTWO_LOWER;
      break;

    case STEPTWO_LOWER:
      if (allRunDone) {
        moveTo(pathX[0], pathY[0], PEN_DOWN);
        drawStep = STEPTHREE_DRAW;
        currentSegment = 0;
        segmentStep = 0;
        lastDrawTime = millis();
      }
      break;

    case STEPTHREE_DRAW:
      if (allRunDone) {
        if (millis() - lastDrawTime < drawInterval) { return; }
        lastDrawTime = millis();
        float t = 1.0 * segmentStep / segmentStepCount;
        float X = pathX[currentSegment] + (pathX[currentSegment + 1] - pathX[currentSegment]) * t;
        float Y = pathY[currentSegment] + (pathY[currentSegment + 1] - pathY[currentSegment]) * t;
        moveTo(X, Y, PEN_DOWN);
        segmentStep++;
        if (segmentStep > segmentStepCount) {
          currentSegment++;
          segmentStep = 0;
        }
        if (currentSegment >= pathCount - 1) {
          moveTo(pathX[currentSegment], pathY[currentSegment], PEN_UP);
          drawStep = STEPFOUR_FINISH;
          drawState = DRAW_IDLE;
          Serial.println("Draw finished !");
        }
      }
    case STEPFOUR_FINISH:
      break;
  }
}

void makeline() {  //数据未测量
  startDraw();
  pathCount = 2;
  pathX[0] = 50;
  pathY[0] = 50;
  pathX[1] = 70;
  pathY[1] = 70;
}

void makeN() {  //数据未测量
  startDraw();
  pathCount = 4;
  pathX[0] = -10;
  pathY[0] = 10;
  pathX[1] = -10;
  pathY[1] = 30;
  pathX[2] = 10;
  pathY[2] = 10;
  pathX[3] = 10;
  pathY[3] = 30;
}


void pauseDraw() {  //暂停
  if (drawState != DRAW_RUNNING) {
    Serial.println("Wrong");
    return;
  }
  drawState = DRAW_PAUSED;
  Serial.println("Paused");
}

void resumeDraw() {  //恢复
  if (drawState != DRAW_PAUSED) {
    Serial.println("Wrong");
    return;
  }
  drawState == DRAW_RUNNING;
  lastDrawTime=millis();
  Serial.println("resumed");
}

void stopDraw() {  //结束
  if (drawState == DRAW_IDLE) {
    Serial.println("Wrong");
    return;
  }
  pathCount = 0;
  currentSegment = 0;
  segmentStep = 0;
  drawState = DRAW_IDLE;
  drawStep=STEPFOUR_FINISH;
  moveTo(0,0,PEN_UP);
  Serial.println("stopped");
}