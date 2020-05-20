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

import dbus

from advertisement import Advertisement
from service import Application, Service, Characteristic, Descriptor
from PCA9685 import PCA9685
from MotorDriver import MotorDriver
import time
import RPi.GPIO as GPIO

GATT_CHRC_IFACE = "org.bluez.GattCharacteristic1"

SVC_UUID = "00000001-710e-4a5b-8d75-3e5b444bc3cf"
RX_UUID  = "00000002-710e-4a5b-8d75-3e5b444bc3cf"
TX_UUID  = "00000003-710e-4a5b-8d75-3e5b444bc3cf"

class TankoAdvertisement(Advertisement):

    def __init__(self, index):
        Advertisement.__init__(self, index, "peripheral")
        self.add_local_name("tankopi")
        self.add_service_uuid(SVC_UUID);
        self.include_tx_power = True

class TankoService(Service):

    def __init__(self, index):
        self.start_pwm()
        self.start_dc_motors()
        value = [
        dbus.Byte(0x5A),
        dbus.Byte(0x00),
        dbus.Byte(100),
        dbus.Byte(0x5A),
        dbus.Byte(0x5A),
        dbus.Byte(0x5A),
        dbus.Byte(0x5A),
        dbus.Byte(0x5A),
        dbus.Byte(100),
        dbus.Byte(100)]
        self.move_motors(value)

        Service.__init__(self, index, SVC_UUID, True)
        self.rxCharacteristic = RxCharacteristic(self)
        self.txCharacteristic = TxCharacteristic(self)
        self.add_characteristic(self.rxCharacteristic)
        self.add_characteristic(self.txCharacteristic)

    def start_pwm(self):
        self.pwm = PCA9685(0x40, debug=False)
        self.pwm.setPWMFreq(50)

    def start_dc_motors(self):
        self.motorDriver = MotorDriver()

    def move_motors(self, value):
        print("---")
        self.value = value
        # Move servo motors
        for channel in range(0, 8):
            i = value[channel]
            self.pwm.setServoAngle(channel,i)
            print("Channel %d: %d" % (channel, i))

        # self.rxCharacteristic.send_update()

        # Move DC motors
        m1 = value[8]
        m2 = value[9]
        self.motorDriver.move(m1 - 100, m2 - 100)

class RxCharacteristic(Characteristic):

    def __init__(self, service):
        self.notifying = False

        Characteristic.__init__(
                self, 
                RX_UUID,
                ["notify"],
                service)

    def send_update(self):
        self.PropertiesChanged(GATT_CHRC_IFACE, {"Value": self.service.value}, [])

    def StartNotify(self):
        if self.notifying:
            return
        self.notifying = True
        self.send_update()

    def StopNotify(self):
        self.notifying = False

    def ReadValue(self, options):
        return self.service.value

class TxCharacteristic(Characteristic):

    def __init__(self, service):
        Characteristic.__init__(
                self,
                TX_UUID,
                ["write-without-response"],
                service)

    def WriteValue(self, value, options):
        self.service.move_motors(value)

app = Application()
app.add_service(TankoService(0))
app.register()

adv = TankoAdvertisement(0)
adv.register()

try:
    app.run()
except KeyboardInterrupt:
    app.quit()
