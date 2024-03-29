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
const double maxSpeed = 255;
const double minSpeed = 60;

void setup(){
  // motors setup
  pinMode(PWMA,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(PWMB,OUTPUT);
  pinMode(DIRB,OUTPUT);

  XBee.begin(57600);
  Serial.begin(57600);

  Serial.println();  
  Serial.println("================");
  Serial.println("================");
  Serial.println("Starting...");
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
  int sw;
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
      sw = n;
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
  fcs ^= sw;
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
  
  double ref = (((rh << 8) & 0xFF00) | (rl & 0xFF));
  ref = ref / 2;

  // normalize x and y: -1 to 1
  x = (x - ref) / ref;
  y = (y - ref) / ref;

  // compute motor speed
  double ma = maxSpeed * (x + y);
  double mb = maxSpeed * (y - x);

  // limit range of motor values to between -maxSpeed and maxSpeed
  ma = constrain(ma, -maxSpeed, maxSpeed);  
  mb = constrain(mb, -maxSpeed, maxSpeed);

  // min speed threshold for motor A
  if (abs(ma) < minSpeed) ma = 0;

  // min speed threshold for motor B
  if (abs(mb) < minSpeed) mb = 0;

  // control motor A
  digitalWrite(DIRA, (ma >= 0) ? HIGH : LOW);
  analogWrite(PWMA, abs(ma));

  // control motor B
  digitalWrite(DIRB, (mb >= 0) ? HIGH : LOW);
  analogWrite(PWMB, abs(mb));

  Serial.print("a: ");
  Serial.print(ma);
  Serial.print(" b: ");
  Serial.print(mb);
  Serial.println();
}
