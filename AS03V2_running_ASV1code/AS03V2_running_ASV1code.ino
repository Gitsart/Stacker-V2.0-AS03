#include<MobaTools.h>
const byte stepPin =18;
const byte dirPin =19;

#define limitPin 9
const byte senPin= 8;
#define startPin 6
#define collectPin 5

const int stepsperRev = 400;


long  nextPos;
const long targetPos =1800;
 
int count = 0;
int limit_count =0;

bool limswitch =0;

int hOme =0;
int pos =0;
int sTart =0;
int cloth = 0;


int cloth_count = 0;

MoToStepper stepper1(stepsperRev, STEPDIR);
MoToTimer stepperPause;

bool stepperRunning;
bool movingForward;
bool clothDetected;


void setup() {
  Serial.begin(9600);
  stepper1.attach(stepPin,dirPin);
  stepper1.setRampLen(20);
  
  stepperRunning = false;
  movingForward = true;
  clothDetected = false;
  

  pinMode(limitPin, INPUT_PULLUP);
  
 
  pinMode(senPin, INPUT_PULLUP);
  pinMode(collectPin, INPUT_PULLUP);
  homing();
  set_pos();
  
}

void loop() {
  int c = digitalRead(collectPin);
  
  
    while(digitalRead(startPin)==LOW)
    {sTart =1;}

    
    if((hOme ==2)&& (limit_count>1)&&(count ==0)&&(pos=1)&& (sTart ==1))
  {
    int sen = digitalRead(senPin);
    bool sensorFound = false;

    if(sen ==0 || sen==1)
    {
      sensorFound = true;
    }
    
    if(sen==0){
      clothDetected =false;
      motor();
      Serial.print("CLOTHCOUNT:");Serial.println(cloth_count);
  
      }
    }
    if(c==0) {
      collect();
      homing();
      set_pos();
    }
}

void homing() {
  stepper1.setSpeed(800);
  while(digitalRead(limitPin) != HIGH && limit_count==0) { stepper1.rotate(-1);Serial.print(limswitch);Serial.println("Finding END");}

  if(digitalRead(limitPin) == HIGH && limit_count ==0) {stepper1.stop();Serial.println("END POSITION");limit_count+=1;limswitch=HIGH;hOme =1;}
  stepper1.setZero();
  delay(2000);
  stepper1.setSpeed(500);stepper1.moveTo(400);
  limswitch =LOW;
  delay(2000);
  while(digitalRead(limitPin) != HIGH && limit_count==1) { stepper1.rotate(-1);Serial.print(limswitch);Serial.println("Finding END2");}
  if(digitalRead(limitPin) == HIGH && limit_count ==1) {stepper1.stop();Serial.println("END POSITION2");limit_count+=1;limswitch=HIGH;hOme =2;}
  Serial.print("limit_count:");Serial.println(limit_count);
  Serial.print("limswitch:");Serial.println(limswitch);
  Serial.print("hOme:");Serial.println(hOme);
  Serial.print("POS:");Serial.println(pos);
  stepper1.setZero();
  delay(2000);
  }

void set_pos() {
  
  while(hOme ==2 && limit_count >1 && limswitch ==HIGH && pos ==0) {
    
    delay(1000);
    stepper1.setSpeed(600);
    stepper1.moveTo(2200);

    while(stepper1.moving()){Serial.println("MOVING to Pos1");}

    pos =1;
    Serial.println("StartPositionSet, READY TO USE");
    }
    Serial.print("POS:");Serial.println(pos);
}

void motor() {

  if (!stepperRunning && !clothDetected)
  {
    // Set the flag to true when signature is not detected
    clothDetected = true;

    // Move forward
    stepper1.setSpeed(800);
    stepper1.move(-targetPos);
    Serial.println("Moving forward");
    cloth_count++;
    stepperRunning = true;

    while (stepper1.moving())
    {
      stepperPause.setTime(100); // Wait until the forward movement is complete
    }
    delay(500);

    // Move backward
    stepper1.setSpeed(1200);
    stepper1.move(targetPos);
    Serial.println("Moving backward");

    while (stepper1.moving())
    {
      stepperPause.setTime(10); // Wait until the backward movement is complete
    }

    stepperRunning = false; // Reset the flag when the complete oscillation is done
  } 
}
void collect() {
  stepper1.setSpeed(1000);
  stepper1.moveTo(-targetPos);
  stepperPause.setTime(5000);
  cloth_count=0;
  sTart = 0;
  limit_count = 0;
  hOme=0;
  pos=0;
}
