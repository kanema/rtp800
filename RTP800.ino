#include <Servo.h> 
 
Servo servo;

const int motorRight = 5;
const int motorLeft = 6;
const int motorDirRight = 7;
const int motorDirLeft = 8;
const int motorMinSpeed = 70;
const int motorMaxSpeed = 255;
const float motorMaxVoltage = 6.0;

const int echoPinFrontRight = 13;
const int trigPinFrontRight = 12;

const int echoPinFrontLeft = 11;
const int trigPinFrontLeft = 10;

const int echoPinBack = 9;
const int trigPinBack = 4;

int obstacleDistance = 50;
int directionChangeTime = 1000;
int stopTime = 250;
int runTime = 250;

bool stopped = true;

float vPow = 5.0;
float r1 = 100000.0;
float r2 = 10000.0;

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

  Serial.println("--------------------");
  Serial.println("DC VOLTMETER");
  Serial.print("Maximum Voltage: ");
  Serial.print((int)(vPow / (r2 / (r1 + r2))));
  Serial.println("V");
  Serial.println("--------------------");
  Serial.println("");
}

int get_distance(int echoP, int trigP){
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

float getSpeed(int speed = motorMaxSpeed)
{
  float voltageMultiplier = getVoltage() / motorMaxVoltage;
  int result =  motorMaxSpeed / voltageMultiplier;
  Serial.print("speedRequired: ");
  Serial.println(speed);
  Serial.print("speedExecuted: ");
  Serial.println(result);
  Serial.print("voltageMultiplier: ");
  Serial.println(voltageMultiplier);
  Serial.println("-----------------");
  return result;
}

// ------------------------------------------------

void motors(int motor, bool on = true, bool front = true, int speed = motorMaxSpeed){
  speed = getSpeed(speed);
  analogWrite(motor, speed);
  digitalWrite((motor == motorLeft) ? motorDirLeft : motorDirRight, (front) ? HIGH : LOW);
}

void run(){
  motors(motorLeft);
  motors(motorRight);
  delay(runTime);
}

void stop()
{
  motors(motorLeft, false, true, 0);
  motors(motorRight, false, true, 0);
  delay(stopTime);
}

void turnLeft()
{
  stop();
  
  motors(motorLeft, true, false);
  motors(motorRight, false, false, 0);
  delay(directionChangeTime);
}

void turnRight()
{
  stop();
  
  motors(motorRight, true, false);
  motors(motorLeft, false, false, 0);
  delay(directionChangeTime);
}

bool hasFrontRightObstacle()
{
  return get_distance(echoPinFrontRight, trigPinFrontRight) < obstacleDistance;
}

bool hasFrontLeftObstacle()
{
  return get_distance(echoPinFrontLeft, trigPinFrontLeft) < obstacleDistance;
}

bool hasBackObstacle()
{
  return get_distance(echoPinBack, trigPinBack) < obstacleDistance;
}

void loop(){
  
  bool frontRightObstacle = hasFrontRightObstacle();
  bool frontLeftObstacle = hasFrontLeftObstacle();
  bool backObstacle = hasBackObstacle();
  bool frontObstacle = frontRightObstacle || frontLeftObstacle;

  if (backObstacle && frontObstacle) {
    stop();
    Serial.println("no way punk");
  }
  else if (backObstacle) {
    run();
    Serial.println("hasBackObstacle");
  }
  else if (frontRightObstacle) {
    turnLeft();
    Serial.println("hasFrontRightObstacle");
  }
  else if (frontLeftObstacle) {
    turnRight();
    Serial.println("hasFrontLeftObstacle");
  }
  else {
    run();
    Serial.println("run");
  }
}

