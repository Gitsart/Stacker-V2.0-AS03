#include <MobaTools.h>
#include <MoToTimer.h>

const byte stepPin = 18;
const byte dirPin = 19;
const int stepsPerRev = 400;
const long collectPos = 2900;

#define startPin 6  // Pin connected to the push button
#define limitPin 9  // Reference switch
const byte senPin = 8; // Cloth sensor
#define solen 25  // Relay switch pin
int buttonPressed = 0; // To track if the button is pressed
bool motorShouldRun = false;  // To track if the motor should be running
bool clothDetected = false;
bool sPeed1 = false, sPeed2 = false, sPeed3 = false, sPeed4 = false;
int fwdSpeed;
int bwdSpeed;
long int relayTime;
int lim = 0;
int limit_count = 0;
bool limswitch = 0;
uint16_t accelarate;
int hOme = 0;
int pos = 0;

unsigned long forwardStartTime = 0;  // Track the start time of forward motion
bool relayActivated = false;  // Track the relay activation state

MoToStepper stepper1(stepsPerRev, STEPDIR);
MoToTimer timer;

char receivedChar; // Assuming a single character is received at a time

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600); // Initialize Hardware(Display)Serial
  stepper1.attach(stepPin, dirPin);

  pinMode(startPin, INPUT_PULLUP);  // Use internal pull-up resistor
  pinMode(limitPin, INPUT_PULLUP);
  pinMode(senPin, INPUT_PULLUP);
  pinMode(solen, OUTPUT);

  homing();
  set_pos();
}

void loop() {
  // Displaying current settings for debugging
  Serial.print("fwdSpeed = "); Serial.println(fwdSpeed);
  Serial.print("bwdSpeed = "); Serial.println(bwdSpeed);
  Serial.print("accelarate = "); Serial.println(accelarate);
  Serial.print("relayTime = "); Serial.println(relayTime);

  receiveDataFromHMI(); // READ_DATA_from_DISPLAY_CONTINUOUSLY
  lim = digitalRead(limitPin);

  if (lim == 1) {
    stepper1.stop();
  }

  if (digitalRead(startPin) == LOW) {
    delay(50); // Debounce delay
    if (digitalRead(startPin) == LOW) {
      buttonPressed = 1;
    }
  }

  if ((hOme == 2) && (limit_count >= 1) && (pos == 1) && (buttonPressed == 1)) {
    int sen = digitalRead(senPin);
    if (sen == 0 && lim != 1) {
      clothDetected = true;
      motor();
      if (lim == 1) {
        stepper1.stop();
      }
    }
    Serial.print("CLOTH DETECTED: "); Serial.println(clothDetected);
  }
}

void receiveDataFromHMI() {
  while (Serial2.available() > 0) {
    receivedChar = Serial2.read(); // Read a character from serial port
    processDataFromHMI(receivedChar); // Process the received character
  }
}

bool isValidButtonPress(char data) {
  // Check if 'data' corresponds to a valid button press
  return (data >= '1' && data <= '9'); // Adjust based on your valid button press criteria
}

void processDataFromHMI(char data) {
  if (isValidButtonPress(data)) {
    // Process the valid button press
    processButtonPress(data);
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
  Serial.print("limit_count: "); Serial.println(limit_count);
  Serial.print("limswitch: "); Serial.println(limswitch);
  Serial.print("hOme: "); Serial.println(hOme);
  Serial.print("POS: "); Serial.println(pos);
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
  Serial.print("POS: "); Serial.println(pos);
}

void processButtonPress(char button) {
  switch (button) {
    case '1':
      Serial.println("Button 1 pressed");
      fwdSpeed = 340;
      bwdSpeed = 1500;
      relayTime = 5500;
      accelarate = 200;
      sPeed1 = true;
      break;
    case '2':
      Serial.println("Button 2 pressed");
      fwdSpeed = 330;
      bwdSpeed = 1400;
      relayTime = 4500;
      accelarate = 150;
      sPeed2 = true;
      break;
    case '3':
      Serial.println("Button 3 pressed");
      fwdSpeed = 210;
      bwdSpeed = 1000;
      relayTime = 13000;
      accelarate = 100;
      sPeed3 = true;
      break;
    case '4':
      Serial.println("Button 4 pressed");
      fwdSpeed = 160;
      bwdSpeed = 1000;
      relayTime = 15500;
      accelarate = 50;
      sPeed4 = true;
      break;
    case '6':
      Serial.println("Speed RESET");
      fwdSpeed = 0;
      bwdSpeed = 0;
      sPeed1 = false; sPeed2 = false; sPeed3 = false; sPeed4 = false;
      buttonPressed = 0;
      break;
    default:
      Serial.println("Invalid button press");
      break;
  }
}

void motor() {
  if (clothDetected) {
    stepper1.setRampLen(accelarate);
    stepper1.setSpeed(fwdSpeed);
    forwardStartTime = millis();  // Record the start time of forward motion
    relayActivated = false;  // Ensure relay is initially deactivated

    while ((digitalRead(senPin) == 0) && (digitalRead(limitPin) != HIGH)) {
      stepper1.rotate(-1);

      if (millis() - forwardStartTime >= relayTime && !relayActivated) {
        digitalWrite(solen, LOW);
        relayActivated = true;
      }
      if (digitalRead(limitPin) == HIGH) {
        stepper1.stop();
      }
      delay(1);  // Small delay to prevent motor stuttering
    }
    stepper1.rotate(-1);
    stepper1.rotate(0);
    digitalWrite(solen, HIGH);
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
