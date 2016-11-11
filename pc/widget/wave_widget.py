#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from config import gWaveXUnit, gWaveXStep, gWaveYUint, gWaveYStep

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

class FCWaveWidget(QWidget):
    # msMove = pyqtSignal(QPoint, name='msMove')
    msMove = pyqtSignal(['QPoint', 'int'], name = "msMove")

    def __init__(self):
        super(FCWaveWidget, self).__init__() 

        # 存放 下位机采样的数据
        self.mData = {}

        self.setMouseTracking(True) # 鼠标跟踪
        self.setCursor(Qt.BlankCursor) # 隐藏鼠标
        self.mPos = None # 鼠标当前点

    def Append(self, time, frameDict):
        """
        time  为键
        frame 为值
        """
        # 加入一帧 打印一次
        """
        print('%05d:{dmpQuat:%s,accel:%s,gyro:%s,compass:%s,press:%s,accelerator:%s,euler:%s,pid:%s}' % (time, 
            frameDict['dmpQuat'], frameDict['accel'], frameDict['gyro'], frameDict['compass'],
            frameDict['press'], frameDict['accelerator'], frameDict['euler'], frameDict['pid']))
        """
        self.mData[time] = frameDict

    def paintEvent(self, paintEvent):
        painter = QPainter(self) 

        # 清屏
        painter.fillRect(0, 0, self.width(), self.height(), Qt.black)

        # 更新绘制参数
        self.updateParas(painter)

        # 绘制坐标系(生成绘布范围,故必须最先调用)
        # 绘制图例
        # 绘制波形 

        # 绘制鼠标位置十字线
        self.drawMouseCross(painter)

        painter.end() 

    def updateParas(self, painter):
        """
        print(gWaveXUnit)
        print(gWaveXStep)
        print(gWaveYUint)
        print(gWaveYStep)
        """

        # 可绘制区域需要扣除label区域
        xMax = self.width()
        yMax = self.height()
        metrics = painter.fontMetrics()
        leftWidth = metrics.width(gWaveYUint)
        downHeigt = metrics.ascent() + metrics.descent()

        # 可绘制区坐标原点(屏幕坐标原点在左上,几何坐标原点在左下)
        # 可绘制区域左下点坐标(xOrig, yOrig)
        # 可绘制区域右上点坐标(xMax, 0)
        xOrig = leftWidth
        yOrig = yMax - downHeigt
        xLength = xMax - leftWidth
        yLength = yMax - downHeigt

        xLabelNums = yLength / gWaveYStep
        yLabelNums = xLength / gWaveXStep




        # 测试代码
        pen = QPen(Qt.red)
        pen.setWidth(1)
        painter.setPen(pen)
        painter.drawLine(xOrig, yOrig, xMax, 0)

        print(xOrig, yOrig)
        print(xMax, 0)

    def drawMouseCross(self, painter): 
        if None != self.mPos:
            x = self.mPos.x()
            y = self.mPos.y() 
            pen = QPen(Qt.gray)
            pen.setWidth(1)
            pen.setStyle(Qt.DashLine)
            painter.setPen(pen)
            painter.drawLine(x, 0, x, self.height())
            painter.drawLine(0, y, self.width(), y)

    def leaveEvent(self, event):
        self.mPos = None
        self.update()

    # 交互类函数
    def mousePressEvent(self, mouseEvent):
        pass

    def mouseReleaseEvent (self, mouseEvent):
        pass

    def mouseMoveEvent(self, event):
        p = QPoint(event.pos())
        self.mPos = p
        self.msMove.emit(p , self.height())
        self.update()

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCWaveWidget()
    win.show()
    sys.exit(app.exec_())

