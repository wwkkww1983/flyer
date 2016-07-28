#!/usr/bin/env python3
# -*- coding: utf-8 -*-

class FCEuler():
    def __init__(self, theta, phi, psi):
        super(FCEuler, self).__init__()
        self.mTheta = theta
        self.mPhi = phi
        self.mPsi = psi

    def Theta(self):
        return self.mTheta

    def Phi(self):
        return self.mPhi

    def Psi(self):
        return self.mPsi

    def Print(self):
        eulerText = "俯仰角:%+5.4f,横滚角:%+5.4f,偏航角:%+5.4f" % (self.Theta(), self.Phi(), self.Psi())
        print(eulerText)

if __name__ == '__main__':
    euler = FCEuler(10, 20, 30)
    euler.Print()

