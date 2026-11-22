const int switchPin = 2;

unsigned long timer = 0;

bool algorithmEnabled = false;
bool previousAlgorithmState = false;

const int frontLeftIRPin = 3;
const int frontRightIRPin = 4;
const int backLeftIRPin = 5;
const int backRightIRPin = 12;

bool frontLeftBorder = false;
bool frontRightBorder = false;
bool backLeftBorder = false;
bool backRightBorder = false;

const int trigPin = 6;
const int echoPin = 7;

float opponentDistance = -1;

bool opponentDetected = false;

const int maxEngagementDistance = 90;


unsigned long lastUltrasonicTime = 0;

const unsigned long ultrasonicInterval = 60;

const int motorLeftPin1 = 8;
const int motorLeftPin2 = 9;

const int motorRightPin1 = 10;
const int motorRightPin2 = 11;


unsigned long lastDebugTime = 0;

const unsigned long debugInterval = 5000;

String currentState = "STOPPED";

String leftMotorState = "STOPPED";
String rightMotorState = "STOPPED";

const unsigned long retreatDuration = 300;


const unsigned long turnDuration = 250;

enum EdgePhase {
  EDGE_IDLE,
  EDGE_RETREAT,
  EDGE_TURN
};

EdgePhase edgePhase = EDGE_IDLE;

unsigned long edgePhaseStart = 0;

// Escape directions

bool retreatBackward = true;
bool turnClockwise = true;
bool escapeNeedsTurn = false;

void setup() {

  Serial.begin(9600);

  Serial.println();
  Serial.println("================================");
  Serial.println("     SUMO ARC ROBOT STARTED     ");
  Serial.println("================================");


  pinMode(switchPin, INPUT);


  pinMode(frontLeftIRPin, INPUT);
  pinMode(frontRightIRPin, INPUT);

  pinMode(backLeftIRPin, INPUT);
  pinMode(backRightIRPin, INPUT);


  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  digitalWrite(trigPin, LOW);


  pinMode(motorLeftPin1, OUTPUT);
  pinMode(motorLeftPin2, OUTPUT);

  pinMode(motorRightPin1, OUTPUT);
  pinMode(motorRightPin2, OUTPUT);


  stopMotors();

  Serial.println("[SYSTEM] Initialization complete.");
  Serial.println("[SYSTEM] Waiting for start switch...");

}


void loop() {

  bool switchState = digitalRead(switchPin);

  if (switchState == HIGH && !algorithmEnabled) {

    if (timer == 0) {

      timer = millis();

    }

    if (millis() - timer >= 4000) {

      algorithmEnabled = true;

      timer = 0;

    }

  }

  else if (switchState == LOW && algorithmEnabled) {

    if (timer == 0) {

      timer = millis();

    }

    if (millis() - timer >= 2000) {

      algorithmEnabled = false;

      timer = 0;

    }

  }

  else {

    timer = 0;

  }


  if (algorithmEnabled != previousAlgorithmState) {

    if (algorithmEnabled) {

      Serial.println("[SYSTEM] ALGORITHM ENABLED");

    }

    else {

      Serial.println("[SYSTEM] ALGORITHM DISABLED");

      edgePhase = EDGE_IDLE;

      stopMotors();

    }

    previousAlgorithmState = algorithmEnabled;

  }

  frontLeftBorder = digitalRead(frontLeftIRPin) == LOW;

  frontRightBorder = digitalRead(frontRightIRPin) == LOW;

  backLeftBorder = digitalRead(backLeftIRPin) == LOW;

  backRightBorder = digitalRead(backRightIRPin) == LOW;


  bool frontEdge = frontLeftBorder || frontRightBorder;

  bool backEdge = backLeftBorder || backRightBorder;

  bool leftEdge = frontLeftBorder || backLeftBorder;

  bool rightEdge = frontRightBorder || backRightBorder;

  bool anyEdge = (
    frontLeftBorder ||
    frontRightBorder ||
    backLeftBorder ||
    backRightBorder
  );

  if (millis() - lastUltrasonicTime >= ultrasonicInterval) {

    lastUltrasonicTime = millis();

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);

    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);

    digitalWrite(trigPin, LOW);

    unsigned long duration = pulseIn(echoPin, HIGH, 25000);

    if (duration > 0) {

      opponentDistance = duration * 0.0343 / 2.0;

    }

    else {

      opponentDistance = -1;

    }

    opponentDetected = (
      opponentDistance > 0 &&
      opponentDistance <= maxEngagementDistance
    );

  }

  if (algorithmEnabled) {


    if (anyEdge && edgePhase == EDGE_IDLE) {

      if (frontEdge && !backEdge) {

        retreatBackward = true;

      }

      else if (backEdge && !frontEdge) {

        retreatBackward = false;

      }

      else {

        retreatBackward = true;

      }

      if (leftEdge && !rightEdge) {

        turnClockwise = true;

        escapeNeedsTurn = true;

      }

      else if (rightEdge && !leftEdge) {

        turnClockwise = false;

        escapeNeedsTurn = true;

      }

      else {

        escapeNeedsTurn = false;

      }

      edgePhase = EDGE_RETREAT;

      edgePhaseStart = millis();

      Serial.println("[EDGE] Boundary detected!");
      Serial.println("[EDGE] Starting escape manoeuvre.");

    }

    if (edgePhase == EDGE_RETREAT) {

      if (retreatBackward) {

        currentState = "EDGE AVOIDANCE - RETREATING BACKWARD";

        moveBackward();

      }

      else {

        currentState = "EDGE AVOIDANCE - RETREATING FORWARD";

        moveForward();

      }

      if (millis() - edgePhaseStart >= retreatDuration) {

        if (escapeNeedsTurn) {

          edgePhase = EDGE_TURN;

          edgePhaseStart = millis();

        }

        else {

          edgePhase = EDGE_IDLE;

        }

      }

    }

    else if (edgePhase == EDGE_TURN) {

      if (turnClockwise) {

        currentState = "EDGE AVOIDANCE - TURNING CLOCKWISE";

        rotateClockwise();

      }

      else {

        currentState = "EDGE AVOIDANCE - TURNING COUNTERCLOCKWISE";

        rotateCounterClockwise();

      }

      if (millis() - edgePhaseStart >= turnDuration) {

        edgePhase = EDGE_IDLE;

        Serial.println("[EDGE] Escape manoeuvre completed.");

      }

    }

    else if (!anyEdge && edgePhase == EDGE_IDLE) {


      if (opponentDetected) {

        currentState = "OPPONENT DETECTED - ADVANCING";

        moveForward();

      }

      else {

        currentState = "SEARCHING - ROTATING CLOCKWISE";

        rotateClockwise();

      }

    }

  }

  else {

    currentState = "ALGORITHM DISABLED - MOTORS STOPPED";

    edgePhase = EDGE_IDLE;

    stopMotors();

  }

  if (millis() - lastDebugTime >= debugInterval) {

    lastDebugTime = millis();

    Serial.println();
    Serial.println("========== ROBOT STATUS ==========");


    // SYSTEM

    Serial.print("[SYSTEM] Algorithm: ");

    Serial.println(
      algorithmEnabled ? "ENABLED" : "DISABLED"
    );


    Serial.println("--- IR EDGE SENSORS ---");

    Serial.print("[IR] Front Left: ");

    Serial.println(
      frontLeftBorder ? "EDGE DETECTED" : "CLEAR"
    );

    Serial.print("[IR] Front Right: ");

    Serial.println(
      frontRightBorder ? "EDGE DETECTED" : "CLEAR"
    );

    Serial.print("[IR] Back Left: ");

    Serial.println(
      backLeftBorder ? "EDGE DETECTED" : "CLEAR"
    );

    Serial.print("[IR] Back Right: ");

    Serial.println(
      backRightBorder ? "EDGE DETECTED" : "CLEAR"
    );


    Serial.println("--- ULTRASONIC SENSOR ---");

    Serial.print("[ULTRASONIC] Distance: ");

    if (opponentDistance > 0) {

      Serial.print(opponentDistance);

      Serial.println(" cm");

    }

    else {

      Serial.println("NO ECHO");

    }

    Serial.print("[OPPONENT] ");

    Serial.println(
      opponentDetected ? "DETECTED" : "NOT DETECTED"
    );

    Serial.println("--- MOVEMENT DECISION ---");

    Serial.print("[STATE] ");

    Serial.println(currentState);


    Serial.println("--- MOTOR STATUS ---");

    Serial.print("[MOTOR LEFT] ");

    Serial.println(leftMotorState);

    Serial.print("[MOTOR RIGHT] ");

    Serial.println(rightMotorState);


    Serial.println("==================================");

  }

}

void moveForward() {

  digitalWrite(motorLeftPin1, HIGH);
  digitalWrite(motorLeftPin2, LOW);

  digitalWrite(motorRightPin1, HIGH);
  digitalWrite(motorRightPin2, LOW);

  leftMotorState = "FORWARD";
  rightMotorState = "FORWARD";

}

void moveBackward() {

  digitalWrite(motorLeftPin1, LOW);
  digitalWrite(motorLeftPin2, HIGH);

  digitalWrite(motorRightPin1, LOW);
  digitalWrite(motorRightPin2, HIGH);

  leftMotorState = "BACKWARD";
  rightMotorState = "BACKWARD";

}

void rotateClockwise() {

  digitalWrite(motorLeftPin1, HIGH);
  digitalWrite(motorLeftPin2, LOW);

  digitalWrite(motorRightPin1, LOW);
  digitalWrite(motorRightPin2, HIGH);

  leftMotorState = "FORWARD";
  rightMotorState = "BACKWARD";

}


void rotateCounterClockwise() {

  digitalWrite(motorLeftPin1, LOW);
  digitalWrite(motorLeftPin2, HIGH);

  digitalWrite(motorRightPin1, HIGH);
  digitalWrite(motorRightPin2, LOW);

  leftMotorState = "BACKWARD";
  rightMotorState = "FORWARD";

}


void stopMotors() {

  digitalWrite(motorLeftPin1, LOW);
  digitalWrite(motorLeftPin2, LOW);

  digitalWrite(motorRightPin1, LOW);
  digitalWrite(motorRightPin2, LOW);

  leftMotorState = "STOPPED";
  rightMotorState = "STOPPED";

}
