#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct

class FCEuler():
    def __init__(self, theta, phi, psi):
        super(FCEuler, self).__init__()

        # 角度单位
        self.mTheta = theta
        self.mPhi = phi
        self.mPsi = psi

    def Theta(self):
        return self.mTheta

    def Phi(self):
        return self.mPhi

    def Psi(self):
        return self.mPsi

    def __getitem__(self, key):
        if '俯仰角' == key:
            return self.Theta()

        elif '横滚角' == key:
            return self.Phi()

        elif '偏航角' == key:
            return self.Psi()

    def ToBytes(self):
        # 角度单位
        eulerBytes = struct.pack('>fff', self.mTheta, self.mPhi, self.mPsi)
        return eulerBytes

    def __str__(self):
        eulerText = "%+5.4f,%+5.4f,%+5.4f" % (self.mTheta, self.mPhi, self.mPsi)
        return eulerText

if __name__ == '__main__':
    euler = FCEuler(math.pi / 2, math.pi / 3, math.pi / 4)
    print(euler)

