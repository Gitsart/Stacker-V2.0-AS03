#include <MobaTools.h>

const byte stepPin = 18;
const byte dirPin = 19;
const int stepsPerRev = 200;
const long targetPos = 1500;
long nextPos;
int fwdSpeed;
int bwdSpeed;

#define buttonPin 6  // Pin connected to the push button

bool sPeed1 = false, sPeed2 = false, sPeed3 = false;
bool buttonPressed = false;  // To track if the button is pressed
bool motorShouldRun = false; // To track if the motor should be running

MoToStepper myStepper(stepsPerRev, STEPDIR);
MoToTimer stepperPause;
bool stepperRunning;
bool movingForward = true;  // To track the direction of movement

String data_display = "";

void setup() {
  myStepper.attach(stepPin, dirPin);
  myStepper.setRampLen(20);
  stepperRunning = true;

  pinMode(buttonPin, INPUT_PULLUP);  // Use internal pull-up resistor

  Serial2.begin(9600);
  Serial.begin(115200);
  Serial.println("DISPLAY MOTOR TEST");
}

void loop() {
  // Read the state of the push button
  if (digitalRead(buttonPin) == LOW) {
    buttonPressed = true;
  } else {
    buttonPressed = false;
  }

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

  // Start the motor if a speed is set and the button is pressed
  if ((sPeed1 || sPeed2 || sPeed3) && buttonPressed) {
    motorShouldRun = true;
  }

  // Run or stop the motor based on motorShouldRun
  if (motorShouldRun) {
    motor();
  } else {
    myStepper.stop();
  }

}

void send_data(String data_display) {
  // Reset all speed flags
  sPeed1 = sPeed2 = sPeed3 = false;

  if (data_display == "1") {
    Serial.println("setSpeed:100RPM");
    sPeed1 = true;
    fwdSpeed = 1000;
    bwdSpeed = 1500;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  } else if (data_display == "2") {
    Serial.println("setSpeed:150RPM");
    sPeed2 = true;
    fwdSpeed = 1500;
    bwdSpeed = 2000;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  } else if (data_display == "3") {
    Serial.println("setSpeed:200RPM");
    sPeed3 = true;
    fwdSpeed = 2000;
    bwdSpeed = 2500;  // Higher speed for anticlockwise
    motorShouldRun = false; // Reset motor run flag
  } else if (data_display == "6") {
    myStepper.stop();
    sPeed1 = sPeed2 = sPeed3 = false;  // Reset speed flags
    motorShouldRun = false; // Reset motor run flag
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
      if (nextPos == 0) {
        nextPos = targetPos;
        myStepper.setSpeed(fwdSpeed);  // Set speed for clockwise
      } else {
        nextPos = 0;
        myStepper.setSpeed(bwdSpeed);  // Set speed for anticlockwise
      }
      myStepper.moveTo(nextPos);
      stepperRunning = true;
    }
  }
}
