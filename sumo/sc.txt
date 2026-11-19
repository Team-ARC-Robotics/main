
// ==========================================
// SUMO ROBOT
// Arduino + L298N + HC-SR04 + IR EDGE SENSORS
// ==========================================


// ==========================================
// 1. START / STOP SWITCH
// ==========================================

const int switchPin = 2;

unsigned long timer = 0;

bool algorithmEnabled = false;


// ==========================================
// 2. IR EDGE DETECTION
// ==========================================

const int leftIRPin = 3;
const int rightIRPin = 4;
const int frontIRPin = 5;

bool leftBorder = false;
bool rightBorder = false;
bool frontBorder = false;


// ==========================================
// 3. ULTRASONIC DETECTION
// ==========================================

const int trigPin = 6;
const int echoPin = 7;

float opponentDistance = -1;

bool opponentDetected = false;

const int maxEngagementDistance = 90;


// ==========================================
// 4. L298N MOTOR DRIVER
// ==========================================

// LEFT MOTOR
const int motorLeftPin1 = 8;
const int motorLeftPin2 = 9;

// RIGHT MOTOR
const int motorRightPin1 = 10;
const int motorRightPin2 = 11;


// ==========================================
// 5. SETUP
// ==========================================

void setup() {

  // START / STOP SWITCH
  pinMode(switchPin, INPUT);


  // IR SENSORS
  pinMode(leftIRPin, INPUT);
  pinMode(rightIRPin, INPUT);
  pinMode(frontIRPin, INPUT);


  // ULTRASONIC SENSOR
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);


  // MOTOR DRIVER
  pinMode(motorLeftPin1, OUTPUT);
  pinMode(motorLeftPin2, OUTPUT);

  pinMode(motorRightPin1, OUTPUT);
  pinMode(motorRightPin2, OUTPUT);


  // INITIAL MOTOR STATE
  stopMotors();

}


// ==========================================
// 6. MAIN LOOP
// ==========================================

void loop() {


  // ----------------------------------------
  // START / STOP CONTROL
  // ----------------------------------------

  bool switchState = digitalRead(switchPin);


  // SWITCH ON
  if (switchState == HIGH && !algorithmEnabled) {

    if (timer == 0) {
      timer = millis();
    }

    if (millis() - timer >= 4000) {
      algorithmEnabled = true;
      timer = 0;
    }
  }


  // SWITCH OFF
  else if (switchState == LOW && algorithmEnabled) {

    if (timer == 0) {
      timer = millis();
    }

    if (millis() - timer >= 2000) {
      algorithmEnabled = false;
      timer = 0;
    }
  }


  // RESET TIMER
  else {
    timer = 0;
  }


  // ----------------------------------------
  // EDGE DETECTION
  // ----------------------------------------

  leftBorder = digitalRead(leftIRPin) == LOW;

  rightBorder = digitalRead(rightIRPin) == LOW;

  frontBorder = digitalRead(frontIRPin) == LOW;


  // ----------------------------------------
  // ULTRASONIC DETECTION
  // ----------------------------------------

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);

  digitalWrite(trigPin, LOW);

  unsigned long duration = pulseIn(echoPin, HIGH, 25000);

  if (duration > 0) {

    opponentDistance = duration * 0.0343 / 2;

  }
  else {

    opponentDistance = -1;

  }


  // ENGAGEMENT DISTANCE LIMIT

  opponentDetected = (
    opponentDistance > 0 &&
    opponentDistance <= maxEngagementDistance
  );


  // ----------------------------------------
  // MAIN ALGORITHM
  // ----------------------------------------

  if (algorithmEnabled) {


    // PRIORITY 1: EDGE AVOIDANCE

    if (leftBorder || rightBorder || frontBorder) {

      // FRONT EDGE
      if (frontBorder) {

        moveBackward();

      }

      // LEFT EDGE
      else if (leftBorder) {

        rotateClockwise();

      }

      // RIGHT EDGE
      else if (rightBorder) {

        rotateCounterClockwise();

      }

    }


    // PRIORITY 2: ATTACK

    else if (opponentDetected) {

      moveForward();

    }


    // PRIORITY 3: SEARCH

    else {

      rotateClockwise();

    }

  }


  // ----------------------------------------
  // ALGORITHM DISABLED
  // ----------------------------------------

  else {

    stopMotors();

  }

}


// ==========================================
// 7. MOTOR MOVEMENT FUNCTIONS
// ==========================================


// MOVE FORWARD

void moveForward() {

  digitalWrite(motorLeftPin1, HIGH);
  digitalWrite(motorLeftPin2, LOW);

  digitalWrite(motorRightPin1, HIGH);
  digitalWrite(motorRightPin2, LOW);

}


// MOVE BACKWARD

void moveBackward() {

  digitalWrite(motorLeftPin1, LOW);
  digitalWrite(motorLeftPin2, HIGH);

  digitalWrite(motorRightPin1, LOW);
  digitalWrite(motorRightPin2, HIGH);

}


// ROTATE CLOCKWISE

void rotateClockwise() {

  digitalWrite(motorLeftPin1, HIGH);
  digitalWrite(motorLeftPin2, LOW);

  digitalWrite(motorRightPin1, LOW);
  digitalWrite(motorRightPin2, HIGH);

}


// ROTATE COUNTERCLOCKWISE

void rotateCounterClockwise() {

  digitalWrite(motorLeftPin1, LOW);
  digitalWrite(motorLeftPin2, HIGH);

  digitalWrite(motorRightPin1, HIGH);
  digitalWrite(motorRightPin2, LOW);

}


// STOP MOTORS

void stopMotors() {

  digitalWrite(motorLeftPin1, LOW);
  digitalWrite(motorLeftPin2, LOW);

  digitalWrite(motorRightPin1, LOW);
  digitalWrite(motorRightPin2, LOW);

}
