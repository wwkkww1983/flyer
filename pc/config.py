#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys 
import socket
import datetime

_dataFileDir = "E:\\workspace\\flyer\\pc\\data\\" 
_fileName = datetime.datetime.now().strftime('%Y_%m_%d_%H_%M_%S') + ".dat"

gLocalIP = socket.gethostbyname(socket.gethostname()) # 获取本地IP
gLocalPort = 8080
gSaveDataFileFullName = os.path.join(_dataFileDir, _fileName)

gWaveXUnit = '秒'
gWaveXStep = 50
gWaveYUint = '  度  '
gWaveYStep = 30

if __name__ == '__main__': 
    print(gLocalIP)
    print(gLocalPort)
    print(gSaveDataFileFullName)

