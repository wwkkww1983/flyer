#!/usr/bin/env python3

import struct
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

class Flyer():
    def __init__(self):
        self.mUdpSocket = socket(AF_INET, SOCK_DGRAM)

    def __del__(self):
        self.mUdpSocket.close()

    def Send(self, data): 
        self.mUdpSocket.sendto(data, gFcAddr)
        pass

    def SendPrintFrame(self, printfStr):
        typeBytes = struct.pack('>I',  FCFrameType.FramePrintText.value) 

        # 数据域: 字符串 + 填充
        dataBytes = printfStr.encode('utf8')
        length = len(dataBytes)
        fillLength = 4 - length % 4
        dataBytes = dataBytes + gFillByte * fillLength

        # 字符串+填充+crc32
        length = length + fillLength + 4
        lenBytes = struct.pack('>I', length) 
        
        data = typeBytes + lenBytes + dataBytes
        crc32 = FCBaseFrame.CalStm32Crc32(data)
        crc32Bytes = struct.pack('>I', crc32)

        data = data + crc32Bytes
        #FCBaseFrame.PrintBytes(data)
        self.Send(data)

if __name__ == '__main__': 
    flyer = Flyer()
    flyer.SendPrintFrame('123')

