#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile
#include <stdio.h>
#include <string.h>
#include <Servo.h>

unsigned long lastRCtime = 0;
int maxTimeWithoutRC = 2000;

// MOTOR
#define motorA 5
#define motorAIn1 22
#define motorAIn2 23
#define motorB 6
#define motorBIn1 24
#define motorBIn2 25
int moveDirection = 1; // 1 - forward, -1 backward
int motorSpeedA = 0;
int motorSpeedB = 0;
int lastMotorSpeedA = 0;

// Remote controller
#define RH_ASK_ARDUINO_USE_TIMER2;
RH_ASK driver(2000, 11, 12, 10, false);

// Front distance measuring instrument
const int trigPinFront = 52;
const int echoPinFront = 53;
int distance = 100;
// rear distance measuring instrument
const int trigPinRear = 50;
const int echoPinRear = 51;
int distance2 = 100;

// Servo
Servo myservo;
int servoPin = 7;
int initalServoPosition = 60;
int minServoPosition = 30;
int maxServoPosition = 90;
int lastServoPosition = initalServoPosition;
int servoTargetPosition = lastServoPosition;
int servoDelay = 100;


// Lights
const int frontWhiteLeft = 49;
const int frontWhiteRight = 48;
const int rearWhiteRight = 47;
const int rearWhiteLeft = 46;
const int rearRedRight = 45;
const int rearRedLeft = 44;
const int bottomBlue1 = 40;
const int bottomBlue2 = 41;
const int bottomBlue3 = 42;
const int bottomBlue4 = 43;
boolean wasBottomBlueButtonReleased = true;
int lastBottomBlueValue = HIGH;
unsigned long lastBottomBluePushed = 0;
const int rearOrangeLeft = 38;
const int rearOrangeRight = 39;
const int frontOrangeRight = 37;
const int frontOrangeLeft = 36;
int lastLeftOrangeValue = LOW;
bool leftOrangeWinks = false;
unsigned long lastLeftOrangePushed = 0;
unsigned long lastLeftOrangeWink = 0;
boolean wasLeftOrangeReleased = true;
int lastRightOrangeValue = LOW;
bool rightOrangeWinks = false;
unsigned long lastRightOrangePushed = 0;
unsigned long lastRightOrangeWink = 0;
boolean wasRightOrangeReleased = true;
int blinkMillis = 500;
int buttonMillisDelay = 400;


int getServoPosition(int yAxis) {
  int servoPosition;
  if (yAxis < 500) {
    servoPosition = map(yAxis, 500, 0, initalServoPosition, minServoPosition);

  } else if (yAxis > 520) {
    servoPosition = map(yAxis, 520, 1023, initalServoPosition, maxServoPosition);
  } else {
    servoPosition = initalServoPosition;
  }


  servoTargetPosition = setServoLimitPosition(servoPosition);
  return servoTargetPosition;
}

int setServoLimitPosition (int servoPosition) {
  // Confine the range from 10 to 110
  if (servoPosition < minServoPosition) {
    servoPosition = minServoPosition;
  }
  if (servoPosition > maxServoPosition) {
    servoPosition = maxServoPosition;
  }

  return servoPosition;
}


void setServoPosition(bool hardSet = false) {
  if (hardSet || (lastServoPosition != servoTargetPosition)) {
//    myservo.attach(servoPin);
    lastServoPosition = servoTargetPosition;
    myservo.write(lastServoPosition);
    delay(servoDelay);
//    myservo.detach();
  }
}


void setup()
{
  Serial.begin(9600); // Debugging only

  // MOTOR
  pinMode(motorA, OUTPUT);
  pinMode(motorAIn1, OUTPUT);
  pinMode(motorAIn2, OUTPUT);
  pinMode(motorB, OUTPUT);
  pinMode(motorBIn1, OUTPUT);
  pinMode(motorBIn2, OUTPUT);

  // Servo
  myservo.attach(servoPin);
  setServoPosition(true);

  // Front distance measuring instrument
  pinMode(trigPinFront, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinFront, INPUT); // Sets the echoPin as an Input
  // Rear distance measuring instrument
  pinMode(trigPinRear, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPinRear, INPUT); // Sets the echoPin as an Input

  // Lights
  pinMode(frontWhiteLeft, OUTPUT);
  pinMode(frontWhiteRight, OUTPUT);
  pinMode(rearWhiteRight, OUTPUT);
  pinMode(rearWhiteLeft, OUTPUT);
  pinMode(rearRedRight, OUTPUT);
  pinMode(rearRedLeft, OUTPUT);
  pinMode(bottomBlue1, OUTPUT);
  pinMode(bottomBlue2, OUTPUT);
  pinMode(bottomBlue3, OUTPUT);
  pinMode(bottomBlue4, OUTPUT);

  pinMode(rearOrangeLeft, OUTPUT);
  pinMode(rearOrangeRight, OUTPUT);
  pinMode(frontOrangeRight, OUTPUT);
  pinMode(frontOrangeLeft, OUTPUT);

  // Remote controller init
  if (!driver.init()) {
    Serial.println("init failed");
  }
  Serial.println("Setup done");
}

void loop()
{
  uint8_t buf[32];
  uint8_t buflen = sizeof(buf);


  if (driver.recv(buf, &buflen)) // Non-blocking
  {



    int i;
    // Message with a good checksum received, dump it.
    Serial.print("Message: ");
    Serial.println((char*)buf);

    String str = (char*)buf;
    char L = str.length();
    String new_str_array[9];

    int j = 0;
    int index = 0;
    for (int i = 0; i < L; i++) {
      if (index == 8) {
        new_str_array[index] = str.substring(j, i + 1);
        index++; // only to skip to next
      }
      if (str.charAt(i) == '|') {
        new_str_array[index] = str.substring(j, i);
        j = i + 1;
        index++;
      }
    }

    //Direction signal
    int yAxis =  new_str_array[0].toInt();
    getServoPosition(yAxis);


    //Speed signal
    int xAxis =  new_str_array[1].toInt();
    setMotorSpeed(xAxis);

    lastRCtime = millis();

    if (new_str_array[3].toInt() == 0 && millis() > lastBottomBluePushed + buttonMillisDelay) {
      if (wasBottomBlueButtonReleased) {
        wasBottomBlueButtonReleased = false;
        if (lastBottomBlueValue == HIGH) {
          lastBottomBlueValue = LOW;
        } else {
          lastBottomBlueValue = HIGH;
        }

        digitalWrite(bottomBlue1, lastBottomBlueValue);
        digitalWrite(bottomBlue2, lastBottomBlueValue);
        digitalWrite(bottomBlue3, lastBottomBlueValue);
        digitalWrite(bottomBlue4, lastBottomBlueValue);
      }
      lastBottomBluePushed = millis();
    } else {
      wasBottomBlueButtonReleased = true;
    }

    if (new_str_array[4].toInt() == 0 && millis() > lastLeftOrangePushed + buttonMillisDelay) {
      if (wasLeftOrangeReleased) {
        wasLeftOrangeReleased = false;
        leftOrangeWinks = !leftOrangeWinks;
        lastLeftOrangePushed = millis();
      }
    } else {
      wasLeftOrangeReleased = true;
    }

    if (new_str_array[5].toInt() == 0 && millis() > lastRightOrangePushed + buttonMillisDelay) {
      if (wasRightOrangeReleased) {
        wasRightOrangeReleased = false;
        rightOrangeWinks = !rightOrangeWinks;
        lastRightOrangePushed = millis();
      }
    } else {
      wasRightOrangeReleased = true;
    }
  }

  // it stops without signal
  if (millis() > lastRCtime + maxTimeWithoutRC) {
    motorSpeedA = 0;
  }

  if (motorSpeedA > 0) {
    getDistance();

    if (distance < 5) {
      motorSpeedA = 0;
    }
  }

  //Lights
  if (moveDirection > 0) {
    digitalWrite(frontWhiteLeft, HIGH);
    digitalWrite(frontWhiteRight, HIGH);
  } else {
    digitalWrite(frontWhiteLeft, LOW);
    digitalWrite(frontWhiteRight, LOW);
  }

  if (moveDirection < 0) {
    digitalWrite(rearWhiteLeft, HIGH);
    digitalWrite(rearWhiteRight, HIGH);
  } else {
    digitalWrite(rearWhiteLeft, LOW);
    digitalWrite(rearWhiteRight, LOW);
  }

  if (moveDirection == 0) {
    digitalWrite(frontWhiteLeft, LOW);
    digitalWrite(frontWhiteRight, LOW);
    digitalWrite(rearWhiteLeft, LOW);
    digitalWrite(rearWhiteRight, LOW);
    digitalWrite(rearRedLeft, LOW);
    digitalWrite(rearRedRight, LOW);
  } else {
    digitalWrite(rearRedLeft, HIGH);
    digitalWrite(rearRedRight, HIGH);
  }

  if (leftOrangeWinks && lastLeftOrangeWink + blinkMillis < millis()) {
    if (lastLeftOrangeValue == HIGH) {
      lastLeftOrangeValue = LOW;
    } else {
      lastLeftOrangeValue = HIGH;
    }
    lastLeftOrangeWink = millis();

  } else if (!leftOrangeWinks && lastLeftOrangeWink + blinkMillis < millis()){
    lastLeftOrangeValue = LOW;
  }
  digitalWrite(frontOrangeLeft, lastLeftOrangeValue);
  digitalWrite(rearOrangeLeft, lastLeftOrangeValue);

  if (leftOrangeWinks) {

  } else {
    digitalWrite(frontOrangeLeft, LOW);
  }


  if (rightOrangeWinks && lastRightOrangeWink + blinkMillis < millis()) {
    if (lastRightOrangeValue == HIGH) {
      lastRightOrangeValue = LOW;
    } else {
      lastRightOrangeValue = HIGH;
    }
    lastRightOrangeWink = millis();

  }  else if (!rightOrangeWinks && lastRightOrangeWink + blinkMillis < millis()){
    lastRightOrangeValue = LOW;
  }
  digitalWrite(frontOrangeRight, lastRightOrangeValue);
  digitalWrite(rearOrangeRight, lastRightOrangeValue);




  if (lastMotorSpeedA != motorSpeedA) {
    motorSpeedB = motorSpeedA;
    analogWrite(motorA, motorSpeedA); // Send PWM signal to motor A
    analogWrite(motorB, motorSpeedB); // Send PWM signal to motor B
    lastMotorSpeedA = motorSpeedA;
  }

  setServoPosition();

}

void getDistance() {
  int pinTriger;
  int pinEcho;
  if (moveDirection > 0) {
    pinTriger = trigPinFront;
    pinEcho = echoPinFront;
  } else if (moveDirection < 0) {
    pinTriger = trigPinRear;
    pinEcho = echoPinRear;
  } else {
    return;
  }

  digitalWrite(pinTriger, LOW);
  delayMicroseconds(2);
  digitalWrite(pinTriger, HIGH);

  // Sets the trigPin on HIGH state for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(pinTriger, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(pinEcho, HIGH);
  // Calculating the d istance

  distance = duration * 0.034 / 2;
}

void setMotorSpeed(int xAxis) {
  int motorSpeed;

  if (xAxis < 470) {
    moveDirection = -1;
    // Convert the declining X-axis readings from 470 to 0 into increasing 0 to 255 value
    int xMapped = map(xAxis, 470, 0, 0, 255);
    // Move to left - decrease left motor speed, increase right motor speed
    motorSpeed = xMapped;

    digitalWrite(motorAIn2, LOW);
    digitalWrite(motorAIn1, HIGH);
    digitalWrite(motorBIn2, LOW);
    digitalWrite(motorBIn1, HIGH);

  } else if (xAxis > 550) {
    moveDirection = 1;
    // Convert the increasing X-axis readings from 550 to 1023 into 0 to 255 value
    int xMapped = map(xAxis, 550, 1023, 0, 255);
    // Move right - decrease right motor speed, increase left motor speed
    motorSpeed = xMapped;

    digitalWrite(motorAIn1, LOW);
    digitalWrite(motorAIn2, HIGH);
    digitalWrite(motorBIn1, LOW);
    digitalWrite(motorBIn2, HIGH);

  } else {
    motorSpeed = 0;
    moveDirection = 0;
  }

  // Prevent buzzing at low speeds (Adjust according to your motors. My motors couldn't start moving if PWM value was below value of 70)
  if (motorSpeed < 70) {
    motorSpeed = 0;
    moveDirection = 0;
  }
  // Confine the range from 0 to 255
  if (motorSpeed < 0) {
    motorSpeed = 0;
    moveDirection = 0;
  }
  // Confine the range from 0 to 255
  if (motorSpeed > 255) {
    motorSpeed = 255;
  }
  motorSpeedA = motorSpeed;
}
