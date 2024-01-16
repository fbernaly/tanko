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

// How to use the NRF24L01 module with Arduino: https://projecthub.arduino.cc/tmekinyan/how-to-use-the-nrf24l01-module-with-arduino-813957
// Doc: https://nrf24.github.io/RF24/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// nrf24
// pin  9 - CE
// pin 10 - CSN
// pin 11 - MOSI
// pin 12 - MISO
// pin 13 - SCK
RF24 radio(9, 10); 
const byte address[6] = "00001";     
int button_pin = 2;

// Joystick pins
int X_PIN = A2;
int Y_PIN = A0;
int REF_PIN = A4;
int SW_PIN = 5;

void setup()
{
  Serial.begin(9600);
  radio.begin();                 
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN); 
  radio.stopListening();  

  pinMode(SW_PIN, INPUT);
}

void loop()
{
  int x = analogRead(X_PIN);
  int y = analogRead(Y_PIN);
  int sw = digitalRead(SW_PIN);
  int ref = analogRead(REF_PIN);

  y = max(0, ref - y);
  x = min(255, map(x, 0, ref, 0, 255));
  y = min(255, map(y, 0, ref, 0, 255));

  if (sw == 1)
    sw = 0x00;
  else if (sw == 0)
    sw = 0x01;

  // checksum
  int fcs = 0x00;
  fcs ^= (x & 0xFF);
  fcs ^= (y & 0xFF);
  fcs ^= (sw & 0xFF);
  
  char data[5];
  data[0] = 0x7E;
  data[1] = (x & 0xFF);
  data[2] = (y & 0xFF);
  data[3] = (sw & 0xFF);
  data[4] = (fcs & 0xFF);

  radio.write(&data, sizeof(data));

  Serial.print(" x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" sw: ");
  Serial.print(sw);
  Serial.print(" fcs: ");
  Serial.print(fcs);
  Serial.println();
}
