#!/usr/bin/python3

"""Copyright (c) 2020, Francisco Bernal

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import PiMotor

class MotorDriver:

    def __init__(self):
        # Name of Individual DC MOTORS
        self.m1 = PiMotor.Motor("MOTOR1",1)
        self.m2 = PiMotor.Motor("MOTOR2",1)
        # Names for Individual Arrows
        self.ab = PiMotor.Arrow(1)
        self.al = PiMotor.Arrow(2)
        self.af = PiMotor.Arrow(3) 
        self.ar = PiMotor.Arrow(4)

    def move(self, m1Speed, m2Speed):
        print("Robot Moving... %d %d" % (m1Speed, m2Speed))
        self.ab.off()
        self.al.off()
        self.af.off()
        self.ar.off()

        if m1Speed > 0:
            self.m1.forward(m1Speed)
        elif m1Speed < 0:
            self.m1.reverse(-1 * m1Speed)
        else:
            self.m1.stop();

        if m2Speed > 0:
            self.m2.forward(m2Speed)
        elif m2Speed < 0:
            self.m2.reverse(-1 * m2Speed)
        else:
            self.m2.stop();

    def moveForward(self, speed):
        print("Robot Moving Forward")
        self.ab.off()
        self.al.off()
        self.af.on()
        self.ar.off()
        self.m1.forward(speed)
        self.m2.forward(speed)

    def moveBackward(self, speed):
        print("Robot Moving Backward")
        self.ab.on()
        self.al.off()
        self.af.off()
        self.ar.off()
        self.m1.reverse(speed)
        self.m2.reverse(speed)

    def moveLeft(self, speed):
        print("Robot Moving Left")
        self.ab.off()
        self.al.on()
        self.af.off()
        self.ar.off()
        self.m1.forward(speed)
        self.m2.reverse(speed)

    def moveRight(self, speed):
        print("Robot Moving Right")
        self.ab.off()
        self.al.off()
        self.af.off()
        self.ar.on()
        self.m1.reverse(speed)
        self.m2.forward(speed)

    def stop(self):
        print("Robot Stopped")
        self.ab.off()
        self.al.off()
        self.af.off()
        self.ar.off()
        self.m1.stop()
        self.m2.stop()
