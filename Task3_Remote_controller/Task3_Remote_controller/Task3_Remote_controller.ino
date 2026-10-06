int button1 = 4;
int button2 = 5;
int button3 = 6;
int button4 = 7;

char action[3] = { 'A', 'B', 'C' };

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
}

void loop() {
  int currentButton1State = digitalRead(button1);
  int currentButton2State = digitalRead(button2);
  int currentButton3State = digitalRead(button3);
  int currentButton4State = digitalRead(button4);

int i=0;
  if (currentButton1State == LOW && lastButton1State == HIGH){delay(20);}
  if(digitalRead(button1) == LOW){
    Serial.print(action[i]);
    i++;
    if(i>=3){i=0;}
  }
lastButton1State=currentButton1State;
}
