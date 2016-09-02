#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import math
import struct

# 弧度转角度
gRad2Arc = 180 / math.pi

class FCEuler():
    def __init__(self, theta, phi, psi):
        super(FCEuler, self).__init__()
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
        eulerBytes = struct.pack('>fff', self.Theta(), self.Phi(), self.Psi())
        return eulerBytes

    def Print(self):
        eulerText = "俯仰角:%+5.4f,横滚角:%+5.4f,偏航角:%+5.4f" % (self.Theta(), self.Phi(), self.Psi())
        print(eulerText)

if __name__ == '__main__':
    euler = FCEuler(0, 0.785, 1.57)
    euler.Print()

