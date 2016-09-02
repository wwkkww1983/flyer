#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct

class FCPid():
    def __init__(self, theta, phi, psi):
        super(FCPid, self).__init__()
        self.mTheta = theta
        self.mPhi = phi
        self.mPsi = psi

    def ToBytes(self):
        pidBytes = struct.pack('>fff', self.mTheta, self.mPhi, self.mPsi)
        return pidBytes

    def __str__(self):
        pidText = "%+5.4f,%+5.4f,%+5.4f" % (self.mTheta, self.mPhi, self.mPsi)
        return pidText

if __name__ == '__main__':
    euler = FCPid(0, 0.785, 1.57)
    print(euler)

