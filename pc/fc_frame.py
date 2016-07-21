#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

class FCFrame():
    def __init__(self):
        super(FCFrame, self).__init__() 

        self.mType = b'\x00\x00\x00\x00'
        self.mLen = b'\x00\x00\x00\x00'
        self.mData = b''
        self.mCrc32 = b'\x00\x00\x00\x00'

    def Print(self):
        print("type: ", end = '')
        print(self.mType)
        print("len:  ", end = '')
        print(self.mLen)
        print("data: ", end = '')
        print(self.mData)
        print("crc32:", end = '')
        print(self.mCrc32)


if __name__ == '__main__': 
    frame = FCFrame()

    frame.Print()

