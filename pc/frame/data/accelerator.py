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

    def Front(self):
        # 归一化 [-0.5, 0.5]
        return 1.0 * self.mFront / self.mMother - 0.5

    def Right(self):
        return 1.0 * self.mRight / self.mMother - 0.5

    def Back(self):
        return 1.0 * self.mBack / self.mMother - 0.5

    def Left(self):
        return 1.0 * self.mLeft / self.mMother - 0.5

    def __getitem__(self, key):
        if '前' == key:
            return self.Front()

        elif '右' == key:
            return self.Right()

        elif '后' == key:
            return self.Back()

        elif '左' == key:
            return self.Left()

    def ToBytes(self):
        acceleratorBytes = struct.pack('>IIIII', self.mFront, self.mRight, self.mBack, self.mLeft, self.mMother)
        return acceleratorBytes

    def __str__(self):
        acceleratorText = "%d,%d,%d,%d of %d" % (self.mFront, self.mRight, self.mBack, self.mLeft, self.mMother)
        return acceleratorText

if __name__ == '__main__':
    accelerator = FCAccelerator(100, 200, 300, 400, 1000)
    print(accelerator)

