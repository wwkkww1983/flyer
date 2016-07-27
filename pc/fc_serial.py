#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
from serial import Serial
from serial.tools import list_ports

#from PyQt5.QtWidgets import *

# 1bit 起始位 8bit 数据 1bit停止位
g_a_byte_bit_nums = 10

class FCSerial(Serial):
    def __init__(self):
        super(FCSerial, self).__init__() 

    def ReadWithTimeout(self, length): 
        # 设置延迟与读取数据长度相关
        baudrate = self.baudrate
        a_byte_trans_time = 1 / (baudrate/ g_a_byte_bit_nums) 
        
        #self.timeout = a_byte_trans_time * (length + 1) # 留1Bytes余量
        self.timeout = a_byte_trans_time * length 

        # 读取
        return self.read(length)

    @staticmethod
    def ListAllPorts(): 
        portsList = []
        allSerial = list_ports.comports()
        for ser in allSerial:
            portsList.append(ser[0])

        return portsList

if __name__ == '__main__': 
    ser = FCSerial()

