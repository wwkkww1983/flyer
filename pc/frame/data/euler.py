#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import struct

# 弧度转角度
gRad2Arc = 180 / math.pi

class FCEuler():
    def __init__(self, theta, phi, psi):
        super(FCEuler, self).__init__()

        # 弧度单位
        self.mTheta = theta
        self.mPhi = phi
        self.mPsi = psi

    def Theta(self):
        return self.mTheta * gRad2Arc

    def Phi(self):
        return self.mPhi * gRad2Arc

    def Psi(self):
        return self.mPsi * gRad2Arc

    def ToBytes(self):
        # 角度单位
        eulerBytes = struct.pack('>fff', self.Theta(), self.Phi(), self.Psi())
        return eulerBytes

    def __str__(self):
        eulerText = "%+5.4f,%+5.4f,%+5.4f" % (self.mTheta, self.mPhi, self.mPsi)
        return eulerText

if __name__ == '__main__':
    euler = FCEuler(math.pi / 2, math.pi / 3, math.pi / 4)
    print(euler)

