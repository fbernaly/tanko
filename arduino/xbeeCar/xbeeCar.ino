/*
 * Copyright (c) 2020, Francisco Bernal
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include "SoftwareSerial.h"

// RX: Arduino pin 10, XBee pin DOUT.
// TX:  Arduino pin 11, XBee pin DIN
SoftwareSerial XBee(10, 11);

/******************************************************************
 * set pins connected to motors:
 ******************************************************************/
#define PWMB            6
#define DIRB            7
#define PWMA            9
#define DIRA            8

// SPEED
#define LOW_SPEED       60
#define MEDIUM_SPEED    150
#define HIGH_SPEED      255

void setup(){
  // motors setup
  pinMode(PWMA,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(PWMB,OUTPUT);
  pinMode(DIRB,OUTPUT);

  XBee.begin(57600);
  Serial.begin(57600);
}

void loop() {
  if (XBee.available() == 0)
    return;

  int count = 0;
  int header;
  int xh;
  int xl;
  int yh;
  int yl;
  int _sw;
  int rh;
  int rl;
  int _fcs;
  while (XBee.available()) {
    int n = XBee.read();

    if (count == 0)
      header = n;
    else if (count == 1)
      xh = n;
    else if (count == 2)
      xl = n;
    else if (count == 3)
      yh = n;
    else if (count == 4)
      yl = n;
    else if (count == 5)
      _sw = n;
    else if (count == 6)
      rh = n;
    else if (count == 7)
      rl = n;
    else if (count == 8)
      _fcs = n;

    count++;
  }
  
  int fcs = 0x00;
  fcs ^= xh;
  fcs ^= xl;
  fcs ^= yh;
  fcs ^= yl;
  fcs ^= _sw;
  fcs ^= rh;
  fcs ^= rl;

  if (fcs != _fcs) {
    Serial.print("Wrong checksum :( ");
    Serial.print(int(fcs));
    Serial.print(" != ");
    Serial.print(int(_fcs));
    Serial.println();
    return;
  }
  
  double x = ((xh << 8) & 0xFF00) | (xl & 0xFF);
  
  double y = ((yh << 8) & 0xFF00) | (yl & 0xFF);
  
  int sw = _sw & 0x01;
  
  double ref = (((rh << 8) & 0xFF00) | (rl & 0xFF));
  ref = ref / 2;
  
  x = x - ref;
  y = y - ref;
  
  double t = atan2(y, x);
  if (t < 0) {
    t += (PI * 2);
  }
  t = t * 180 / PI;
  
  double speed = sqrt((x * x) + (y * y));
  speed = speed * 255 / ref;
  speed = min(255, speed);
  if (speed < 30)
    speed = 0;
  else if (speed <= 100)
    speed = LOW_SPEED;
  else if (speed <= 200)
    speed = MEDIUM_SPEED;
  else
    speed = HIGH_SPEED;
  
  Serial.print("x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" sw: ");
  Serial.print(sw);
  Serial.print(" ref: ");
  Serial.print(ref);
  Serial.print(" angle: ");
  Serial.print(t);
  Serial.print(" speed: ");
  Serial.print(speed);
  Serial.println();
  
  if (t >= 0 && t <= 15)
    rotateRight(speed);
  else if (t > 15 && t <= 75)
    turnRightFowward(speed);
  else if (t > 75 && t <= 105)
    moveForward(speed);
  else if (t > 105 && t <= 165)
    turnLeftFowward(speed);
  else if (t > 165 && t <= 195)
    rotateLeft(speed);
  else if (t > 195 && t <= 255)
    turnRightBackward(speed);
  else if (t > 255 && t <= 285)
    moveBackward(speed);
  else if (t > 285 && t <= 3455)
    turnLeftBackward(speed);
  else if (t > 345 && t <= 360)
    rotateRight(speed);
}

void moveForward(int speed) {
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, speed);
}

void moveBackward(int speed) {
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, speed);
}

void rotateRight(int speed) {
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, LOW);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, speed);
}

void rotateLeft(int speed) {
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, HIGH);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, speed);
}

void turnRightFowward(int speed) {
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, 0);
}

void turnLeftFowward(int speed) {
  digitalWrite(DIRA, HIGH);
  digitalWrite(DIRB, HIGH);
  analogWrite(PWMA, 0);
  analogWrite(PWMB, speed);
}

void turnRightBackward(int speed) {
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
  analogWrite(PWMA, speed);
  analogWrite(PWMB, 0);
}

void turnLeftBackward(int speed) {
  digitalWrite(DIRA, LOW);
  digitalWrite(DIRB, LOW);
  analogWrite(PWMA, 0);
  analogWrite(PWMB, speed);
}
