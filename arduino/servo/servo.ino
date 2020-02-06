/*
 Controlling a servo position
*/

#include <Servo.h>

Servo myservo;  // create servo object to control a servo

void setup() {
  Serial.begin(9600);
  Serial.println("Servo test!");

  myservo.attach(9);  // attaches the servo on pin 9 to the servo object
}

void rotate(int angle) {
  Serial.print("angle: ");
  Serial.println(angle);
  myservo.write(angle);   // sets the servo position according to the scaled value

  delay(2000);            // waits for the servo to get there
}

void loop() {
  rotate(0);
  rotate(90);
  rotate(180);
}
