#include <Arduino.h>

const int xPins[] = {2, 3, 4, 5};     // IN1, IN2, IN3, IN4  X
const int yPins[] = {A0, A1, A2, A3}; // IN1, IN2, IN3, IN4  Y
const int zPins[] = {9, 10, 11, 12};  // IN1, IN2, IN3, IN4  Z


const uint8_t stepPhases[] = { 0b0001, 0b0010, 0b0100, 0b1000 };
int xStepIdx = 0; int yStepIdx = 0; int zStepIdx = 0;

bool absoluteMode = true; 
float currentX = 0; float currentY = 0;       
const float stepsPerMm = 43.478; 

const int zStepsTravel = 238; 

void makeStepX(bool dir) {
  if (dir) xStepIdx++; else xStepIdx--;
  if (xStepIdx > 3) xStepIdx = 0; if (xStepIdx < 0) xStepIdx = 3;
  for (int i = 0; i < 4; i++) digitalWrite(xPins[i], (stepPhases[xStepIdx] >> i) & 1);
}

void makeStepY(bool dir) {
  if (dir) yStepIdx++; else yStepIdx--;
  if (yStepIdx > 3) yStepIdx = 0; if (yStepIdx < 0) yStepIdx = 3;
  for (int i = 0; i < 4; i++) digitalWrite(yPins[i], (stepPhases[yStepIdx] >> i) & 1);
}

void makeStepZ(bool dir) {
  if (dir) zStepIdx++; else zStepIdx--;
  if (zStepIdx > 3) zStepIdx = 0; if (zStepIdx < 0) zStepIdx = 3;
  for (int i = 0; i < 4; i++) digitalWrite(zPins[i], (stepPhases[zStepIdx] >> i) & 1);
}

void setup() {
  for (int i = 0; i < 4; i++) {
    pinMode(xPins[i], OUTPUT); pinMode(yPins[i], OUTPUT); pinMode(zPins[i], OUTPUT);
    digitalWrite(xPins[i], LOW); digitalWrite(yPins[i], LOW); digitalWrite(zPins[i], LOW);
  }
  Serial.begin(115200); Serial.println("Grbl 1.1h ['$' for help]");
}

void processSingleCommand(String cmd) {
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd.indexOf("G90") != -1) absoluteMode = true;
  if (cmd.indexOf("G91") != -1) absoluteMode = false;
  if (cmd.startsWith("G92")) { currentX = 0; currentY = 0; return; }


  bool isM3 = (cmd.indexOf("M3") != -1 || cmd.indexOf("M03") != -1);
  int sIdx = cmd.indexOf('S');
  bool isSOn = (sIdx != -1 && cmd.substring(sIdx + 1).toFloat() > 0);

  if (isM3 || isSOn) {
    for (int s = 0; s < zStepsTravel; s++) {
      makeStepZ(true); 
      delay(3);
    }
    for (int i = 0; i < 4; i++) digitalWrite(zPins[i], LOW); 
    return;
  }


  bool isM5 = (cmd.indexOf("M5") != -1 || cmd.indexOf("M05") != -1);
  bool isSOff = (sIdx != -1 && cmd.substring(sIdx + 1).toFloat() == 0);

  if (isM5 || isSOff) {
    for (int s = 0; s < zStepsTravel; s++) {
      makeStepZ(false); 
      delay(3);
    }
    for (int i = 0; i < 4; i++) digitalWrite(zPins[i], LOW); 
    return;
  }


  if (cmd.startsWith("G1") || cmd.startsWith("G0")) {
    float rawX = currentX; float rawY = currentY;
    bool hasX = false; bool hasY = false;

    int xIdx = cmd.indexOf('X'); int yIdx = cmd.indexOf('Y');
    if (xIdx != -1) { rawX = cmd.substring(xIdx + 1).toFloat(); hasX = true; }
    if (yIdx != -1) { rawY = cmd.substring(yIdx + 1).toFloat(); hasY = true; }

    float targetX = absoluteMode ? (hasX ? rawX : currentX) : (hasX ? (currentX + rawX) : currentX);
    float targetY = absoluteMode ? (hasY ? rawY : currentY) : (hasY ? (currentY + rawY) : currentY);

    long stepsToMoveX = abs(targetX - currentX) * stepsPerMm;
    long stepsToMoveY = abs(targetY - currentY) * stepsPerMm;
    long maxSteps = max(stepsToMoveX, stepsToMoveY);

    for (long s = 0; s < maxSteps; s++) {
      if (hasX && s < stepsToMoveX) makeStepX((targetX - currentX) > 0);
      if (hasY && s < stepsToMoveY) makeStepY((targetY - currentY) > 0);
      delay(3); 
    }


    for (int i = 0; i < 4; i++) {
      digitalWrite(xPins[i], LOW);
      digitalWrite(yPins[i], LOW);
    }

    currentX = targetX; currentY = targetY;
  }
}

void loop() {
  if (Serial.available() > 0) {
    char c = Serial.peek();
    if (c == '?') {
      Serial.read();
      Serial.print("<Idle|MPos:"); Serial.print(currentX, 3); Serial.print(",");
      Serial.print(currentY, 3); Serial.println(",0.000|Bf:15,128|FS:0,0>");
      return;
    }

    String inputLine = Serial.readStringUntil('\n');
    inputLine.trim(); inputLine.toUpperCase();
    if (inputLine.length() == 0) return;

    int semiColonIdx = inputLine.indexOf(';');
    if (semiColonIdx != -1) {
      while (inputLine.length() > 0) {
        int idx = inputLine.indexOf(';');
        if (idx == -1) {
          processSingleCommand(inputLine); break;
        } else {
          String subCmd = inputLine.substring(0, idx);
          processSingleCommand(subCmd);
          inputLine = inputLine.substring(idx + 1);
        }
      }
    } else {
      processSingleCommand(inputLine);
    }
    Serial.println("ok");
  }
}
