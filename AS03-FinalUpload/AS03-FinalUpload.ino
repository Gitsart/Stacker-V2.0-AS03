#include <MobaTools.h>

const byte stepPin = 18;
const byte dirPin = 19;
const int stepsPerRev = 400;
const long collectPos = 3000;

#define startPin 6  // Pin connected to the push button
#define limitPin 9  // Reference switch
const byte senPin = 8; // Cloth sensor
#define solen 25  // Relay switch pin

bool sPeed1 = false, sPeed2 = false, sPeed3 = false;
bool buttonPressed = false;  // To track if the button is pressed
bool motorShouldRun = false;  // To track if the motor should be running
bool clothDetected = false;

int fwdSpeed;
int bwdSpeed;

int limit_count = 0;
bool limswitch = 0;

int hOme = 0;
int pos = 0;

unsigned long forwardStartTime = 0;  // Track the start time of forward motion
bool relayActivated = false;  // Track the relay activation state

MoToStepper stepper1(stepsPerRev, STEPDIR);

String data_display = "";

void setup() {
  Serial2.begin(9600);
  Serial.begin(115200);
  Serial.println("DISPLAY MOTOR TEST");
  stepper1.attach(stepPin, dirPin);
  stepper1.setRampLen(60);

  pinMode(startPin, INPUT_PULLUP);  // Use internal pull-up resistor
  pinMode(limitPin, INPUT_PULLUP);
  pinMode(senPin, INPUT_PULLUP);
  pinMode(solen, OUTPUT);
  
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
  if ((hOme == 2) && (limit_count >= 1) && (pos == 1) && buttonPressed && (sPeed1 || sPeed2 || sPeed3)) {
    int sen = digitalRead(senPin);
    if (sen == 0) {
      clothDetected = true;
      motor();
    }
    Serial.print("CLOTH DETECTED:"); Serial.println(clothDetected);
  }
}

void send_data(String data_display) {
  // Reset all speed flags
  sPeed1 = sPeed2 = sPeed3 = false;

  if (data_display == "1") {
    Serial.println("setSpeed:100RPM");
    sPeed1 = true;
    fwdSpeed = 150;
    bwdSpeed = 1800;  // Higher speed for reverse stroke
    motorShouldRun = false;  // Reset motor run flag
  } else if (data_display == "2") {
    Serial.println("setSpeed:150RPM");
    sPeed2 = true;
    fwdSpeed = 250;
    bwdSpeed = 1800;  // Higher speed for reverse stroke
    motorShouldRun = false;  // Reset motor run flag
  } else if (data_display == "3") {
    Serial.println("setSpeed:200RPM");
    sPeed3 = true;
    fwdSpeed = 400;
    bwdSpeed = 1800;  // Higher speed for reverse stroke
    motorShouldRun = false;  // Reset motor run flag
  } else if (data_display == "4") {
    Serial.println("setSpeed:200RPM");
    sPeed3 = true;
    fwdSpeed = 550;
    bwdSpeed = 1800;  // Higher speed for reverse stroke
    motorShouldRun = false;  // Reset motor run flag
  } else if (data_display == "6") {
    stepper1.stop();
    sPeed1 = sPeed2 = sPeed3 = false;  // Reset speed flags
    motorShouldRun = false;  // Reset motor run flag
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
    stepper1.setSpeed(600);
    stepper1.moveTo(collectPos);

    while (stepper1.moving()) {
      Serial.println("MOVING to Pos1");
    }

    pos = 1;
    Serial.println("StartPositionSet, READY TO USE");
  }
  Serial.print("POS:"); Serial.println(pos);
}

void motor() {
  if (clothDetected) {
    // Move forward while sensor is detecting cloth (value is 0)
    stepper1.setSpeed(fwdSpeed);
    forwardStartTime = millis();  // Record the start time of forward motion
    relayActivated = false;  // Ensure relay is initially deactivated
    
    while ((digitalRead(senPin) == 0) && (digitalRead(limitPin) != HIGH)) {
      stepper1.rotate(-1);
      
      // Activate relay if 5 seconds have passed since the forward motion started
      if (millis() - forwardStartTime >= 5000 && !relayActivated) {
        digitalWrite(solen, HIGH);
        relayActivated = true;
      }
      
      delay(1);  // Small delay to prevent motor stuttering
    }

    stepper1.stop();
    digitalWrite(solen, LOW);  // Deactivate relay
    delay(500);

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
