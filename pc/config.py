#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys 
import math
import socket
import datetime

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

# 弧度转角度
gRad2Arc = 180 / math.pi
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
gXTimePerPixelRate = 0.1
_xPixelPerTimeRate = 1 / gXTimePerPixelRate
gXPixelPerTimemsRate = _xPixelPerTimeRate / 1000
gWaveYStep = 30
gWaveYUint = '  度  '
#gWaveYUint = ' PID  '
# 默认为 1度/像素
gYAnglePerPixelRate = 1
gYPixelPerAngleRate = 1 / gYAnglePerPixelRate
# 默认为 1pid/像素
gYPidPerPixelRate = 1
gYPixelPerPidRate = 1 / gYPidPerPixelRate
# 默认为 0.003/像素, FCAccelerator获取的油门已经归一化 属于[-0.5, 0.5]
gYAcceleratorPerPixelRate = 0.003
gYPixelPerAcceleratorRate = 1 / gYAcceleratorPerPixelRate
# 默认为 0.x/像素
gYAccelPerPixelRate = 0.1
gYPixelPerAccelRate = 1 / gYAccelPerPixelRate

# 坐标轴颜色线宽
gWaveAxesColor = Qt.darkGray
gWaveAxesWidth = 1

# 颜色线宽
gWaveConfig = [
        ('加计X', Qt.red, 1),
        ('加计Y', Qt.green, 1),
        ('加计Z', Qt.blue, 1),

        #('俯仰角', Qt.red, 1),
        #('横滚角', Qt.green, 1),
        #('偏航角', Qt.blue, 1),

        #('俯仰PID', Qt.green, 1),
        #('横滚PID', Qt.green, 1),
        #('偏航PID', Qt.blue, 1),

        # 油门暂未实现
        #('前', Qt.blue, 1),
        #('右', Qt.green, 1),
        #('后', Qt.yellow, 1),
        #('左', Qt.yellow, 1)
        ]

# 按键自动设置的步长
gKeyStep = 0.01 

# 下位机就绪打印
gFlyerInitDoneStr = '初始化完成,进入主循环'

if __name__ == '__main__': 
    print(gLocalIP)
    print(gLocalPort)
    print(gSaveDataFileFullName)

