#!/usr/bin/env python3

import struct
import time
from socket import *

# 帧
from frame.base import FCBaseFrame
from frame.up import FCUpFrame

# 配置
from config import gLocalIP
from config import gLocalPort

# 类型
from frame.type import FCFrameType
from frame.base import gFillByte

gFcAddr = (gLocalIP, gLocalPort)
gCrcLen = 4
gFillUint = 4
gStartTime = time.perf_counter() 

class Flyer():
    def __init__(self):
        self.mUdpSocket = socket(AF_INET, SOCK_DGRAM)

    def __del__(self):
        self.mUdpSocket.close()

    def Send(self, data): 
        print("send to %s:%d:" % gFcAddr, end = '')
        self.mUdpSocket.sendto(data, gFcAddr)

    def SendPrintFrame(self, printfStr):
        typeBytes = struct.pack('>I',  FCFrameType.FramePrintText.value) 

        now = time.perf_counter() - gStartTime
        nowInt = int(now * 5000) & 0xffffffff # 5000表示精度为 1/5000 s
        # 1970以来的ms数
        dataBytes = struct.pack('>I', nowInt)
        # 数据域: 字符串 + 填充
        dataBytes = dataBytes + printfStr.encode('utf8')
        length = len(dataBytes)
        fillLength = gFillUint - length % gFillUint
        dataBytes = dataBytes + gFillByte * fillLength

        # 字符串+填充+crc32
        length = length + fillLength + gCrcLen
        lenBytes = struct.pack('>I', length) 
        
        data = typeBytes + lenBytes + dataBytes + b'\x00' * gCrcLen
        upFrame = FCUpFrame(data)
        wordList = upFrame.GetCrc32WordList()
        crc32 = FCBaseFrame.CalStm32Crc32(wordList)
        crc32Bytes = struct.pack('>I', crc32)
        data = data[0:-gCrcLen] + crc32Bytes

        upFrame = FCUpFrame(data)
        self.Send(data)
        print('[%05d],%s' % (nowInt, printfStr), end = '')

if __name__ == '__main__': 
    flyer = Flyer()
    flyer.SendPrintFrame('你好！\r\n')
    flyer.SendPrintFrame('数字:123\r\n')
    flyer.SendPrintFrame('英文:abc\r\n')
    flyer.SendPrintFrame('混搭:世界，您好！abc,123\r\n')

