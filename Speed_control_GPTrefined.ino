#include <MobaTools.h>

const byte stepPin = 9;
const byte dirPin = 8;
const int stepsPerRev = 200;
const long targetPos = 2000;
long nextPos;
int fwdSpeed;

#define ledPin A15

bool sPeed1 = false, sPeed2 = false, sPeed3 = false, lEd = false;

MoToStepper myStepper(stepsPerRev, STEPDIR);
MoToTimer stepperPause;
bool stepperRunning;

String data_display = "";

void setup() {
  myStepper.attach(stepPin, dirPin);
  myStepper.setRampLen(20);
  stepperRunning = true;

  pinMode(ledPin, OUTPUT);

  Serial2.begin(9600);
  Serial.begin(115200);
  Serial.println("DISPLAY MOTOR TEST");
}

void loop() {
  if (Serial2.available()) {
    data_display = "";
    delay(30);
    while (Serial2.available()) {
      char ch = Serial2.read();
      if (ch != '\n' && ch != '\r') { // filter out newline and carriage return
        data_display += ch;
      }
    }
    send_data(data_display);
  }

  // Simplify stepper control
  if (sPeed1 || sPeed2 || sPeed3) {
    myStepper.setSpeed(fwdSpeed);
    motor();
  } else {
    myStepper.stop();
  }

  // LED control
  digitalWrite(ledPin, lEd ? HIGH : LOW);
}

void send_data(String data_display) {
  sPeed1 = sPeed2 = sPeed3 = false;

  if (data_display == "1") {
    Serial.println("setSpeed:100RPM");
    sPeed1 = true;
    fwdSpeed = 1000;
    lEd = false;
  } else if (data_display == "2") {
    Serial.println("setSpeed:150RPM");
    sPeed2 = true;
    fwdSpeed = 1500;
    lEd = false;
  } else if (data_display == "3") {
    Serial.println("setSpeed:200RPM");
    sPeed3 = true;
    fwdSpeed = 2000;
    lEd = false;
  } else if (data_display == "5") {
    myStepper.setSpeed(2500);
    myStepper.rotate(1);
    lEd = true;
  } else if (data_display == "RESET") {
    myStepper.stop();
    lEd = true;
    Serial.println("RESETTING...");
  } else {
    Serial.println("Incorrect Data");
  }
}

void motor() {
  if (stepperRunning) {
    // Wait till stepper has reached target, then set pause time
    if (!myStepper.moving()) {
      // stepper has reached target, start pause
      stepperPause.setTime(1000);
      stepperRunning = false;
    }
  } else {
    // stepper doesn't move, wait till stepperPause time expires
    if (stepperPause.expired()) {
      // stepperPause time expired. Start stepper in opposite direction
      nextPos = (nextPos == 0) ? targetPos : 0;
      myStepper.moveTo(nextPos);
      stepperRunning = true;
    }
  }
}
