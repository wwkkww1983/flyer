#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct

class FCAccel():
    def __init__(self, x, y, z):
        super(FCAccel, self).__init__()
        self.mX = x
        self.mY = y
        self.mZ = z

    def __getitem__(self, key):
        if '加计X' == key:
            return self.mX

        elif '加计Y' == key:
            return self.mY

        elif '加计Z' == key:
            return self.mZ

    def ToBytes(self):
        accelBytes = struct.pack('>fff', self.mX, self.mY, self.mZ)
        return accelBytes

    def __str__(self):
        pidText = "%+5.4f,%+5.4f,%+5.4f" % (self.mX, self.mY, self.mZ)
        return pidText

if __name__ == '__main__':
    accel = FCAccel(0, 0.785, 1.57)
    print(accel)

