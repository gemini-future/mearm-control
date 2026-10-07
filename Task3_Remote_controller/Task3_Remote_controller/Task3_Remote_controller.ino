#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

int button1 = 4;
int button2 = 5;
int button3 = 6;
int button4 = 7;

//按钮1
char action[3] = { 'A', 'B', 'C' };
int group = 0;
unsigned long interval = 20;
unsigned long lastButton1Time = 0;
//按钮2
int i=0;
char state[2]={'Q','W'};
unsigned long lastButton2Time = 0;
//按钮3
unsigned long lastButton3Time = 0;
//按钮4
unsigned long lastButton4Time = 0;

int lastButton1State = HIGH;
int lastButton2State = HIGH;
int lastButton3State = HIGH;
int lastButton4State = HIGH;

void setup() {
  pinMode(button1, INPUT_PULLUP);
  pinMode(button2, INPUT_PULLUP);
  pinMode(button3, INPUT_PULLUP);
  pinMode(button4, INPUT_PULLUP);
  Serial.begin(9600);
  mySerial.begin(9600);
}

void loop() {
  int currentButton1State = digitalRead(button1);
  int currentButton2State = digitalRead(button2);
  int currentButton3State = digitalRead(button3);
  int currentButton4State = digitalRead(button4);

  unsigned long now = millis();
  //按钮1
  if (currentButton1State != lastButton1State) { lastButton1Time = now; }
  if (now - lastButton1Time > interval) {
    if (currentButton1State != lastButton1State) {
      if (digitalRead(button1) == LOW) {
        mySerial.println(action[group]);
        Serial.println(action[group]);
        group++;
        if (group >= 3) { group = 0; }
      }
      lastButton1State = currentButton1State;
    }
  }
//按钮2
  if (currentButton2State != lastButton2State) { lastButton2Time = now; }
  if (now - lastButton2Time > interval) {
    if (currentButton2State != lastButton2State) {
      if (digitalRead(button2) == LOW) {
        mySerial.println(state[i]);
        Serial.println(state[i]);
        i++;
        if (i >= 2) { i = 0; }
      }
      lastButton2State = currentButton2State;
    }
  }
  //按钮3
  if (currentButton3State != lastButton3State) { lastButton3Time = now; }
  if (now - lastButton3Time > interval) {
    if (currentButton3State != lastButton3State) {
      if (digitalRead(button3) == LOW) {
        mySerial.println('P');
        Serial.println('P');
      }
      lastButton3State = currentButton3State;
    }
  }
  //按钮4
  if (currentButton4State != lastButton4State) { lastButton4Time = now; }
  if (now - lastButton4Time > interval) {
    if (currentButton4State != lastButton4State) {
      if (digitalRead(button4) == LOW) {
        mySerial.println('E');
        Serial.println('E');
      }
      lastButton4State = currentButton4State;
    }
  }
}
