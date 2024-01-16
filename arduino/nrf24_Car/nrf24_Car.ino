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

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// nrf24
// pin  5 - CE
// pin 10 - CSN
// pin 11 - MOSI
// pin 12 - MISO
// pin 13 - SCK
RF24 radio(5, 10); // CE, CSN
const byte address[6] = "00001";

/******************************************************************
 * set pins connected to motors:
 ******************************************************************/
#define PWMB            6
#define DIRB            7
#define PWMA            9
#define DIRA            8

// SPEED
const double maxSpeed = 255;

void setup(){
  // motors setup
  pinMode(PWMA,OUTPUT);
  pinMode(DIRA,OUTPUT);
  pinMode(PWMB,OUTPUT);
  pinMode(DIRB,OUTPUT);

  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);   //Setting the address at which we will receive the data
  radio.setPALevel(RF24_PA_MAX);       //You can set this as minimum or maximum depending on the distance between the transmitter and receiver.
  radio.startListening();              //This sets the module as receiver

  Serial.println();  
  Serial.println("================");
  Serial.println("================");
  Serial.println("Starting...");
}

void loop() {
  if (!radio.available()) return;

  char data[5] = "";                 //Saving the incoming data
  radio.read(&data, sizeof(data));    //Reading the data

  int _x = data[1] & 0xFF;
  int _y = data[2] & 0xFF;
  int sw = data[3] & 0xFF;
  int _fcs = data[4] & 0xFF;

  Serial.print(" _x: ");
  Serial.print(_x);
  Serial.print(" _y: ");
  Serial.print(_y);
  Serial.print(" sw: ");
  Serial.print(sw);
  Serial.print(" _fcs: ");
  Serial.print(_fcs);

  int fcs = 0x00;
  fcs ^= (_x & 0xFF);
  fcs ^= (_y & 0xFF);
  fcs ^= (sw & 0xFF);

  if (fcs != _fcs) {
    Serial.print("Wrong checksum :( ");
    Serial.print(int(fcs));
    Serial.print(" != ");
    Serial.print(int(_fcs));
    Serial.println();
    return;
  }

  double x = double(_x);
  double y = double(_y);
  double ref = 255 / 2;

  // normalize x and y: -1 to 1
  x = double(int(((x - ref) / ref) * 10))/10;
  y = double(int(((y - ref) / ref) * 10))/10;

  // compute motor speed
  double ma = maxSpeed * (x + y);
  double mb = maxSpeed * (y - x);

  // limit range of motor values to between -maxSpeed and maxSpeed
  ma = constrain(ma, -maxSpeed, maxSpeed);  
  mb = constrain(mb, -maxSpeed, maxSpeed);

  // control motor A
  digitalWrite(DIRA, (ma >= 0) ? HIGH : LOW);
  analogWrite(PWMA, abs(ma));

  // control motor B
  digitalWrite(DIRB, (mb >= 0) ? HIGH : LOW);
  analogWrite(PWMB, abs(mb));

  Serial.print(" x: ");
  Serial.print(x);
  Serial.print(" y: ");
  Serial.print(y);
  Serial.print(" ma: ");
  Serial.print(ma);
  Serial.print(" mb: ");
  Serial.print(mb);

  Serial.println();

  delay(10);
}
