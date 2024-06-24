#include <MobaTools.h>

const byte stepPin = 18;
const byte dirPin = 19;
const int stepsPerRev = 400;
const long collectPos = 2800;

#define startPin 6  // Pin connected to the push button
#define limitPin 9  // Reference switch
const byte senPin = 8; // Cloth sensor
#define solen 25  // Relay switch pin
int buttonPressed = 0; // To track if the button is pressed
bool motorShouldRun = false;  // To track if the motor should be running
bool clothDetected = false;

int fwdSpeed = 600;
int bwdSpeed = 1000;

int limit_count = 0;
bool limswitch = 0;

int hOme = 0;
int pos = 0;

unsigned long forwardStartTime = 0;  // Track the start time of forward motion
bool relayActivated = false;  // Track the relay activation state

MoToStepper stepper1(stepsPerRev, STEPDIR);

void setup() {
  Serial.begin(115200);
  stepper1.attach(stepPin, dirPin);

  pinMode(startPin, INPUT_PULLUP);  // Use internal pull-up resistor
  pinMode(limitPin, INPUT_PULLUP);
  pinMode(senPin, INPUT_PULLUP);
  pinMode(solen, OUTPUT);

  homing();
  set_pos();
}

void loop() {
  Serial.print("buttonPressed:" + buttonPressed);
  if (digitalRead(startPin) == LOW) {
    buttonPressed = 1;
  }

  if ((hOme == 2) && (limit_count >= 1) && (pos == 1) && (buttonPressed == 1)) {
    int sen = digitalRead(senPin);
    if (sen == 0) {
      clothDetected = true;
      motor();
    }
    Serial.print("CLOTH DETECTED:"); Serial.println(clothDetected);
  }


}

void homing() {
  stepper1.setRampLen(400);
  stepper1.setSpeed(600);
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
  stepper1.setSpeed(500);
  stepper1.moveTo(400);
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
    stepper1.setRampLen(200);
    stepper1.setSpeed(600);
    stepper1.moveTo(collectPos);

    while (stepper1.moving()) {
      Serial.println("MOVING to Pos1");
    }

    pos = 1;
    Serial.println("StartPositionSet, READY TO USE");
    digitalWrite(solen, HIGH);
    buttonPressed = 0;
  }
  Serial.print("POS:"); Serial.println(pos);
}

void motor() {
  if (clothDetected) {
    stepper1.setRampLen(200);
    // Move forward while sensor is detecting cloth (value is 0)
    stepper1.setSpeed(fwdSpeed);
    forwardStartTime = millis();  // Record the start time of forward motion
    relayActivated = false;  // Ensure relay is initially deactivated

    while ((digitalRead(senPin) == 0) && (digitalRead(limitPin) != HIGH)) {
      stepper1.rotate(-1);

      if (millis() - forwardStartTime >= 5500 && !relayActivated) {
        digitalWrite(solen, LOW);
        relayActivated = true;
      }
      if (digitalRead(limitPin) == HIGH) {
        stepper1.stop();
      }

      delay(1);  // Small delay to prevent motor stuttering
    }
    stepper1.stop();
    delay(500);
    digitalWrite(solen, HIGH);
    // Move backward to the predefined collecting position
    stepper1.setSpeed(bwdSpeed);
    stepper1.moveTo(collectPos);
    Serial.println("Moving backward to collecting position");

    while (stepper1.moving()) {
      // No delay here to ensure continuous movement
    }

    motorShouldRun = false;  // Reset the flag when the complete oscillation is done
    clothDetected = false;  // Reset cloth detection flag
    relayActivated = false;  // Reset relay activation flag
  }
}
