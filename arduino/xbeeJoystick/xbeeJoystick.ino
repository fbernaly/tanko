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

// RX: Arduino pin 2, XBee pin DOUT.
// TX:  Arduino pin 3, XBee pin DIN
int DOUT_PIN = 2;
int DIN_PIN = 3;
SoftwareSerial XBee(DOUT_PIN, DIN_PIN);

// Joystick pins
int X_PIN = A2;
int Y_PIN = A0;
int REF_PIN = A4;
int SW_PIN = 5;

void setup()
{
  XBee.begin(57600);
  Serial.begin(57600);

  pinMode(SW_PIN, INPUT);
}

void loop()
{
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);
  char sw = digitalRead(SW_PIN);
  int ref = analogRead(REF_PIN);

  y = ref - y;

  if (sw == 1)
    sw = 0x00;
  else if (sw == 0)
    sw = 0x01;

  char xh = (x >> 8) & 0xF;
  char xl = x & 0xFF;
  char yh = (y >> 8) & 0xF;
  char yl = y & 0xFF;
  char rh = (ref >> 8) & 0xF;
  char rl = ref & 0xFF;

  // checksum
  char fcs = 0x00;
  fcs ^= xh;
  fcs ^= xl;
  fcs ^= yh;
  fcs ^= yl;
  fcs ^= sw;
  fcs ^= rh;
  fcs ^= rl;

  char data[9];
  data[0] = 0x7E;
  data[1] = xh;
  data[2] = xl;
  data[3] = yh;
  data[4] = yl;
  data[5] = sw;
  data[6] = rh;
  data[7] = rl;
  data[8] = fcs;
  
  XBee.write(data, 9);

  Serial.print("x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" sw: ");
  Serial.print(int(sw));
  Serial.print(" ref: ");
  Serial.println(ref);
  
  delay(50);
}
