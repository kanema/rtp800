#include <Servo.h> 
 
Servo servo;

const int D_STOP = 0;
const int D_LEFT = 1;
const int D_RIGHT = 2;
const int D_FRONT = 3;
int currentDirection = D_STOP;

const int motorRight = 5;
const int motorLeft = 6;
const int motorDirRight = 7;
const int motorDirLeft = 8;

const int motorMinSpeed = 160;
const int motorMaxSpeed = 200;
const float motorMaxVoltage = 6.0;

const int echoPinFrontRight = 13;
const int trigPinFrontRight = 12;

const int echoPinFrontLeft = 11;
const int trigPinFrontLeft = 10;

const int echoPinBack = 9;
const int trigPinBack = 4;

bool frontRightObstacle;
bool frontLeftObstacle;
bool backObstacle;
bool frontObstacle;

int sunDelay = 10 * 1000 * 1;
int lightSearchTime = 100;
int obstacleDistance = 60;
int directionChangeTime = 100;
int stopTime = 50;
int runTime = 50;

float vPow = 5.0;
float r1 = 100000.0;
float r2 = 10000.0;

const int NO_LIGHT = 0;
const int LEFT_LIGHT = 1;
const int RIGHT_LIGHT = 2;
const int FRONT_LIGHT = 3;
const int GOOD_LIGHT = 4;

const int LDR_MIN_DIFF = 30;
const int LDR_MIN_LUX = 800;
const int LDR_REQUIRED_LUX = 300;

int ldr = NO_LIGHT;

int ldrRPin = A1;
int ldrRValue = 0;

int ldrLPin = A2;
int ldrLValue = 0;

// ------------------------------------------------

void setup(){
  Serial.begin(9600);
  
  // drivers
  pinMode(motorLeft, OUTPUT);
  pinMode(motorRight, OUTPUT);
  pinMode(motorDirLeft, OUTPUT);
  pinMode(motorDirRight, OUTPUT);

  // sonar
  pinMode(echoPinFrontRight, INPUT);
  pinMode(trigPinFrontRight, OUTPUT);
  pinMode(echoPinFrontLeft, INPUT);
  pinMode(trigPinFrontLeft, OUTPUT);
  pinMode(echoPinBack, INPUT);
  pinMode(trigPinBack, OUTPUT);

  Serial.println("start");
}

int getDistance(int echoP, int trigP){
  long duration, distance;
  digitalWrite(trigP, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(trigP, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(trigP, LOW);
  duration = pulseIn(echoP, HIGH);
  distance = (duration/2) / 29.1;
  return distance;
}

float getVoltage()
{
  float v = (analogRead(0) * vPow) / 1024.0;
  return v / (r2 / (r1 + r2));
}

float getSpeed(int speed)
{
  if (speed == 0) {
    return speed;
  }
  float voltageMultiplier = getVoltage() / motorMaxVoltage;
  int result =  speed / voltageMultiplier;
  // Serial.println(result);
  return result;
}

// ------------------------------------------------

void motors(int motor, bool on = true, bool front = true, int speed = motorMaxSpeed){
  speed = getSpeed(speed);
  analogWrite(motor, speed);
  digitalWrite((motor == motorLeft) ? motorDirLeft : motorDirRight, (front) ? HIGH : LOW);
}

void checkDirection(int direction){
  bool isAnotherDirection = direction != currentDirection;
  if (isAnotherDirection) {
    if (direction != D_STOP) {
      stop();
    }
    currentDirection = direction;
  }
}

void run(int time){
  checkDirection(D_FRONT);
  motors(motorLeft);
  motors(motorRight);
  delay(time);
}

void run()
{
  run(runTime);
}

void stop(int time)
{
  checkDirection(D_STOP);
  motors(motorLeft, false, true, 0);
  motors(motorRight, false, true, 0);
  delay(time);
}

void stop()
{
  stop(stopTime);
}

void turnLeft(int time)
{
  checkDirection(D_LEFT);
  
  motors(motorLeft, true, false);
  motors(motorRight, false, false, 0);
  delay(time);
}

void turnLeft()
{
  turnLeft(directionChangeTime);
}

void turnRight(int time)
{
  checkDirection(D_RIGHT);
  
  motors(motorRight, true, false);
  motors(motorLeft, false, false, 0);
  delay(time);
}

void turnRight()
{
  turnRight(directionChangeTime);
}

bool hasFrontRightObstacle()
{
  return getDistance(echoPinFrontRight, trigPinFrontRight) < obstacleDistance;
}

bool hasFrontLeftObstacle()
{
  return getDistance(echoPinFrontLeft, trigPinFrontLeft) < obstacleDistance;
}

bool hasBackObstacle()
{
  return getDistance(echoPinBack, trigPinBack) < obstacleDistance;
}

int getLight()
{
  ldrRValue = analogRead(ldrRPin);
  ldrLValue = analogRead(ldrLPin);
  int ldrRLDiffValue = abs(ldrRValue - ldrLValue);
  bool hasDiff = ldrRLDiffValue > LDR_MIN_DIFF;
  bool hasLight = (ldrRValue < LDR_MIN_LUX) && (ldrLValue < LDR_MIN_LUX);
  bool hasGoodLight = (ldrRValue < LDR_REQUIRED_LUX) && (ldrLValue < LDR_REQUIRED_LUX);

  if (hasGoodLight) {
    return GOOD_LIGHT;
  }

  if ( ! hasLight) {
    return NO_LIGHT;
  }

  if (hasDiff) {
    if (ldrRValue < ldrLValue) {
      return RIGHT_LIGHT;
    } else {
      return LEFT_LIGHT;
    }
  }
  
  return FRONT_LIGHT;
}

void walk()
{ 
  if (backObstacle && frontObstacle) {
    stop();
  }
  else if (backObstacle) {
    run();
  }
  else if (frontRightObstacle) {
    turnLeft();
  }
  else if (frontLeftObstacle) {
    turnRight();
  }
  else {
    run();
  }
}

void sunbath() {
  Serial.println("sunbath ^_^");
  delay(sunDelay);
  Serial.println("back to work. :(");
}

int cicles = 0;
int sleepCicle = 100;
int sleepTime = 10 * 1000 * 1;

void goToSleep()
{
  Serial.println("sleep time zZzZ");
  delay(sleepTime);
  cicles = 0;
}

bool checkSleep() {
  bool needSleep = cicles >= sleepCicle;
  
  if (needSleep) {
    goToSleep();
  }
  
  cicles++;
}

void loop(){

  checkSleep();
  
  frontRightObstacle = hasFrontRightObstacle();
  frontLeftObstacle = hasFrontLeftObstacle();
  backObstacle = hasBackObstacle();
  frontObstacle = frontRightObstacle || frontLeftObstacle;
  
  ldr = getLight();

  if (ldr == GOOD_LIGHT) {
    sunbath();
  }
  
  if (ldr == LEFT_LIGHT && ! frontLeftObstacle) {
    turnLeft(lightSearchTime);
  }
  if (ldr == RIGHT_LIGHT && ! frontRightObstacle) {
    turnRight(lightSearchTime);
  }
  if (ldr == FRONT_LIGHT && ! frontObstacle) {
    run(lightSearchTime);
  }
  else {
    walk();
  }
  
}

