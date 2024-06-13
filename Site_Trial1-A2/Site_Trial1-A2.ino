#include <MobaTools.h>

const byte stepPin = 18;
const byte dirPin = 19;
const int stepsPerRev = 400;
const long targetPos = 2200;
long nextPos;
int fwdSpeed;
int bwdSpeed;

#define startPin 6  // Pin connected to the push button
#define limitPin 9 //reference
const byte senPin = 8; //cloth

bool sPeed1 = false, sPeed2 = false, sPeed3 = false;
bool buttonPressed = false;  // To track if the button is pressed
bool motorShouldRun = false; // To track if the motor should be running
bool movingForward = true;  // To track the direction of movement
bool clothDetected;

int count = 0;
int limit_count = 0;

bool limswitch = 0;

int hOme = 0;
int pos = 0;
int sTart = 0;
int cloth = 0;


int cloth_count = 0;


MoToStepper stepper1(stepsPerRev, STEPDIR);
MoToTimer stepperPause;

String data_display = "";

void setup() {
  Serial2.begin(9600);
  Serial.begin(115200);
  Serial.println("DISPLAY MOTOR TEST");
  stepper1.attach(stepPin, dirPin);
  stepper1.setRampLen(50);
  motorShouldRun = true;
  clothDetected = false;

  pinMode(startPin, INPUT_PULLUP);  // Use internal pull-up resistor

  pinMode(limitPin, INPUT_PULLUP);


  pinMode(senPin, INPUT_PULLUP);
  homing();
  set_pos();
}

void loop() {
  // Check for serial input
  if (Serial2.available()) {
    data_display = "";
    delay(30);
    while (Serial2.available()) {
      char ch = Serial2.read();
      if (ch != '\n' && ch != '\r') {  // filter out newline and carriage return
        data_display += ch;
      }
    }
    send_data(data_display);
  }

  // Read the state of the push button
  if (digitalRead(startPin) == LOW) {
    buttonPressed = true;
  }
  Serial.print("Button:"); Serial.println(buttonPressed);
  Serial.print("Speed1:"); Serial.println(sPeed1);
  Serial.print("Speed2:"); Serial.println(sPeed2);
  Serial.print("Speed3:"); Serial.println(sPeed3);

  // Start the motor if a speed is set and the button is pressed
  if ((hOme == 2) && (limit_count >= 1) && (count == 0) && (pos = 1) && (buttonPressed) && (sPeed1 || sPeed2 || sPeed3))
  {

    motorShouldRun = true;
    int sen = digitalRead(senPin);
    bool sensorFound = false;

    if (sen == 0 || sen == 1)
    {
      sensorFound = true;
    }

    if ((sen == 0)) {
      delay(1400);
      clothDetected = false;
      motor();
    }
    Serial.print("CLOTHCOUNT:"); Serial.println(cloth_count);

  }
}


// Run or stop the motor based on motorShouldRun
//  if ((motorShouldRun)) {
//    motor();
//  } else {
//    stepper1.stop();
//  }


void send_data(String data_display) {
  // Reset all speed flags
  sPeed1 = sPeed2 = sPeed3 = false;

  if (data_display == "1") {
    Serial.println("setSpeed:100RPM");
    sPeed1 = true;
    fwdSpeed = 300;
    bwdSpeed = 2200;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  } else if (data_display == "2") {
    Serial.println("setSpeed:150RPM");
    sPeed2 = true;
    fwdSpeed = 1200;
    bwdSpeed = 2200;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  } else if (data_display == "3") {
    Serial.println("setSpeed:200RPM");
    sPeed3 = true;
    fwdSpeed = 1600;
    bwdSpeed = 2200;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  } else if (data_display == "4") {
    Serial.println("setSpeed:200RPM");
    sPeed3 = true;
    fwdSpeed = 2000;
    bwdSpeed = 2200;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  }
  else if (data_display == "6") {
    stepper1.stop();
    sPeed1 = sPeed2 = sPeed3 = false;  // Reset speed flags
    motorShouldRun = false; // Reset motor run flag
    buttonPressed = false;
    Serial.println("RESETTING...");
  } else {
    Serial.println("Incorrect Data");
  }
}


void homing() {
  stepper1.setSpeed(800);
  while (digitalRead(limitPin) != HIGH && limit_count == 0) {
    stepper1.rotate(-1);
    Serial.print(limswitch);
    Serial.println("Finding END");
  }

  if (digitalRead(limitPin) == HIGH && limit_count == 0) {
    stepper1.stop();
    Serial.println("END POSITION");
    limit_count += 1;
    limswitch = HIGH;
    hOme = 1;
  }
  stepper1.setZero();
  delay(2000);
  stepper1.setSpeed(500); stepper1.moveTo(400);
  limswitch = LOW;
  delay(2000);
  while (digitalRead(limitPin) != HIGH && limit_count == 1) {
    stepper1.rotate(-1);
    Serial.print(limswitch);
    Serial.println("Finding END2");
  }
  if (digitalRead(limitPin) == HIGH && limit_count == 1) {
    stepper1.stop();
    Serial.println("END POSITION2");
    limit_count += 1;
    limswitch = HIGH;
    hOme = 2;
  }
  Serial.print("limit_count:"); Serial.println(limit_count);
  Serial.print("limswitch:"); Serial.println(limswitch);
  Serial.print("hOme:"); Serial.println(hOme);
  Serial.print("POS:"); Serial.println(pos);
  stepper1.setZero();
  delay(2000);
}

void set_pos() {

  while (hOme == 2 && limit_count > 1 && limswitch == HIGH && pos == 0) {

    delay(1000);
    stepper1.setSpeed(600);
    stepper1.moveTo(3000);

    while (stepper1.moving()) {
      Serial.println("MOVING to Pos1");
    }

    pos = 1;
    Serial.println("StartPositionSet, READY TO USE");
  }
  Serial.print("POS:"); Serial.println(pos);
}

void motor() {

  if (motorShouldRun && !clothDetected)
  {
    // Set the flag to true when signature is not detected
    clothDetected = true;

    // Move forward
    stepper1.setSpeed(fwdSpeed);
    stepper1.move(-targetPos);
    Serial.println("Moving forward");
    cloth_count++;
    motorShouldRun = true;

    while (stepper1.moving())
    {
      stepperPause.setTime(100); // Wait until the forward movement is complete
    }
    delay(500);

    // Move backward
    stepper1.setSpeed(bwdSpeed);
    stepper1.move(targetPos);
    Serial.println("Moving backward");

    while (stepper1.moving())
    {
      stepperPause.setTime(10); // Wait until the backward movement is complete
    }

    motorShouldRun = false; // Reset the flag when the complete oscillation is done
  }
}
