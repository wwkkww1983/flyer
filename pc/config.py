#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys 
import socket
import datetime

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

_dataFileDir = "E:\\workspace\\flyer\\pc\\data\\" 
_fileName = datetime.datetime.now().strftime('%Y_%m_%d_%H_%M_%S') + ".dat"

gLocalIP = socket.gethostbyname(socket.gethostname()) # 获取本地IP
gLocalPort = 8080
gSaveDataFileFullName = os.path.join(_dataFileDir, _fileName)

gWaveXUnit = '秒'
gWaveXStep = 50
# X轴 每个像素表示100ms(0.1s)
gXTimePixelRate = 0.1

# Y轴需要成对使用 默认为角度 0.1度/像素
gWaveYUint = '  度  '
gYAnglePixelRate = 0.1
#gWaveYUint = ' PID  '
#gYPidPixelRate = 1

gWaveYStep = 30

gWaveAxesColor = Qt.darkGray
gWaveAxesWidth = 1

if __name__ == '__main__': 
    print(gLocalIP)
    print(gLocalPort)
    print(gSaveDataFileFullName)

