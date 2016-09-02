#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct

class FCAccelerator():
    def __init__(self, front, right, back, left, mother):
        super(FCAccelerator, self).__init__()
        self.mFront = front
        self.mRight = right
        self.mBack = back
        self.mLeft = left
        self.mMother = mother

    def ToBytes(self):
        acceleratorBytes = struct.pack('>IIIII', self.mFront, self.mRight, self.mBack, self.mLeft, self.mMother)
        return acceleratorBytes

    def __str__(self):
        acceleratorText = "%d,%d,%d,%d of %d" % (self.mFront, self.mRight, self.mBack, self.mLeft, self.mMother)
        return acceleratorText

if __name__ == '__main__':
    accelerator = FCAccelerator(100, 200, 300, 400, 1000)
    print(accelerator)

