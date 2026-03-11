// ------------------- Motor Driver Pins -------------------
#define RMF 2
#define RMR 3
#define LMF 0
#define LMR 1

int motorSpeed = 150;

// ------------------- Set Motor Speeds -------------------
void setMotors(int LM, int RM) {
  LM = constrain(LM, -255, 255);
  RM = constrain(RM, -255, 255);

  if (LM >= 0) {
    analogWrite(LMF, LM);
    analogWrite(LMR, 0);
  } else {
    analogWrite(LMF, 0);
    analogWrite(LMR, -LM);
  }

  if (RM >= 0) {
    analogWrite(RMF, RM);
    analogWrite(RMR, 0);
  } else {
    analogWrite(RMF, 0);
    analogWrite(RMR, -RM);
  }
}

void stopMotors() {
  setMotors(0, 0);
}

void setup() {
  pinMode(RMF, OUTPUT);
  pinMode(RMR, OUTPUT);
  pinMode(LMF, OUTPUT);
  pinMode(LMR, OUTPUT);

  stopMotors();
  Serial.begin(115200);
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "forward") {
      setMotors(motorSpeed, motorSpeed);
    } 
    else if (cmd == "backward") {
      setMotors(-motorSpeed, -motorSpeed);
    } 
    else if (cmd == "left") {
      setMotors(-motorSpeed, motorSpeed);
    } 
    else if (cmd == "right") {
      setMotors(motorSpeed, -motorSpeed);
    } 
    else if (cmd == "stop") {
      stopMotors();
    }
  }
}
