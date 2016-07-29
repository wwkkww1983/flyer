#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from math import atan2, asin

from algo.euler import FCEuler

class FCQuat():
    def __init__(self, q0, q1, q2, q3):
        super(FCQuat, self).__init__()
        self.mQ0 = q0
        self.mQ1 = q1
        self.mQ2 = q2
        self.mQ3 = q3

    def ToEuler(self):
        theta =  atan2(self.mQ2 * self.mQ3 + self.mQ0 * self.mQ1, self.mQ0 * self.mQ0 + self.mQ3 * self.mQ3 - 0.5)
        phi   = -asin(2 * (self.mQ1 * self.mQ3 - self.mQ0 * self.mQ2))
        psi   =  atan2(self.mQ1 * self.mQ2 + self.mQ0 * self.mQ3, self.mQ0 * self.mQ0 + self.mQ1 * self.mQ1 - 0.5)
        euler =  FCEuler(theta, phi, psi)
        return euler

    def ToString(self):
        quatText = "%+5.4f,%+5.4f,%+5.4f,%+5.4f" % (self.mQ0, self.mQ1, self.mQ2, self.mQ3)
        return quatText

    def Print(self):
        print(self.ToString())

if __name__ == '__main__':
    quat = FCQuat(1, 0, 0, 0)
    quat.Print()

