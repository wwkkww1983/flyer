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

# 通信ip:port
gLocalIP = socket.gethostbyname(socket.gethostname()) # 获取本地IP
gLocalPort = 8080



# 本地备份数据
gSaveDataFileFullName = os.path.join(_dataFileDir, _fileName)



# 绘图配置
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

# 坐标轴颜色线宽
gWaveAxesColor = Qt.darkGray
gWaveAxesWidth = 1

# 颜色线宽
gWaveConfig = [
        ('俯仰角', Qt.red, 1),
        ('横滚角', Qt.green, 1),
        ('偏航角', Qt.blue,  1),

        ('俯仰PID', None, 1),
        ('横滚PID', None, 1),
        ('偏航PID', None, 1),

        ('前油门', None, 1),
        ('右油门', None, 1),
        ('后油门', None, 1),
        ('左油门', None, 1)]


if __name__ == '__main__': 
    print(gLocalIP)
    print(gLocalPort)
    print(gSaveDataFileFullName)

