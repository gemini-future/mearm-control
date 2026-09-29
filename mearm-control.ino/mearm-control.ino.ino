#include <Servo.h>

Servo base;
Servo rArm;
Servo lArm;
Servo claw;
//定义各舵机初始角度
int initialangle_b=90;
int initialangle_r=90;
int initialangle_l=90;
int initialangle_c=90;

void setup() {
  base.attach(9);//底座连9号引脚
  lArm.attach(8);//前臂连8号引脚
  rArm.attach(7);//后臂连7号引脚
  claw.attach(6);//爪子连6号引脚
  Serial.begin(9600);
  Serial.println("please input servoName and angle");
  base.write(90);
  lArm.write(90);
  rArm.write(90);
  claw.write(90);
}

void loop() {
 chooseServoRun();
}



void chooseServoRun(){
  int i;
  if(Serial.available()>0){
    char servoName=Serial.read();
    Serial.print("servoName is");
    Serial.print(servoName);
    Serial.print(",");
    int angle=Serial.parseInt();
    while(Serial.available()>0){Serial.read();}
    switch(servoName){
      case 'b'://底座
      //控制速度
      if(initialangle_b<angle){
        for(i=initialangle_b;i<=angle;i++){
        base.write(i);
        delay(15);
        }
      }
      else{
        for(i=initialangle_b;i>=angle;i--){
        base.write(i);
        delay(15);
        }
      }
      initialangle_b=angle;//记录现在的角度
        Serial.print("The base angle is");
        Serial.println(angle);
        break;
      case 'r'://后臂
      //控制速度
      if(initialangle_r<angle){
        for(i=initialangle_r;i<=angle;i++){
        rArm.write(i);
        delay(15);
        }
      }
      else{
        for(i=initialangle_r;i>=angle;i--){
        rArm.write(i);
        delay(15);
        }
      }
      initialangle_r=angle;//记录现在的角度
        Serial.print("The rArm angle is");
        Serial.println(angle);
        break;
      case 'l'://前臂
      //控制速度
      if(initialangle_l<angle){
        for(i=initialangle_l;i<=angle;i++){
        lArm.write(i);
        delay(15);
        }
      }
      else{
        for(i=initialangle_l;i>=angle;i--){
        lArm.write(i);
        delay(15);
        }
      }
      initialangle_l=angle;//记录现在的角度
        Serial.print("The lArm angle is");
        Serial.println(angle);
        break;
      case 'c'://爪子
      //控制速度
      if(initialangle_c<angle){
        for(i=initialangle_c;i<=angle;i++){
        claw.write(i);
        delay(15);
        }
      }
      else{
        for(i=initialangle_c;i>=angle;i--){
        claw.write(i);
        delay(15);
        }
      }
      initialangle_c=angle;//记录现在的角度
        Serial.print("The claw angle is");
        Serial.println(angle);
        break;
    }

  }

}


