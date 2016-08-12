#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

class FCWaveWidget(QWidget):
    #msMove = pyqtSignal(QPoint, name='msMove')
    msMove = pyqtSignal(['QPoint', 'int'], name = "msMove")

    def __init__(self):
        super(FCWaveWidget, self).__init__() 

        #
        self.mConfig = {
                '横坐标单位' : '秒',
                '横坐标步长' : 50,
                '横坐标短线边界' : 8,

                '纵坐标单位' : '  度  ',
                '纵坐标步长' : 30,
                '纵坐标短线边界' : 32,
                }
        
        # TODO: 使用参数传入
        self.mXPhyStep = 100  # 每个像素点为100ms间隔
        self.mYPhyRange = 10 # 纵坐标角度范围 [-5, 5]

        self.setMouseTracking(True) # 鼠标跟踪
        self.setCursor(Qt.BlankCursor) # 隐藏鼠标
        self.mPos = None # 鼠标当前点

    def paintEvent(self, paintEvent):
        painter = QPainter(self) 

        # 清屏
        painter.fillRect(0, 0, self.width(), self.height(), Qt.black)
        # 绘制坐标系
        self.drawAxes(painter)
        # 绘制鼠标位置十字线
        self.drawMouseCross(painter)

        painter.end() 

    def drawAxes(self, painter): 
        xMax = self.width()
        yMax = self.height()

        metrics = painter.fontMetrics() 
        xUnit = self.mConfig['横坐标单位']
        xStep = self.mConfig['横坐标步长']
        xShortLineMagin = self.mConfig['横坐标短线边界']

        yUnit = self.mConfig['纵坐标单位']
        yStep = self.mConfig['纵坐标步长']
        yShortLineMagin = self.mConfig['纵坐标短线边界']

        #print(xUnit)
        #print(yUnit)
        leftWidth = metrics.width(yUnit)
        downHeight = metrics.ascent() + metrics.descent()
        #print(leftWidth)
        #print(downHeight)

        # 绘制坐标系
        pen = QPen(Qt.red)
        pen.setWidth(1)
        painter.setPen(pen)

        # 坐标原点
        # +/-1避免被截断
        x = leftWidth + 1
        y = yMax - 1
        #painter.rotate(270)
        painter.drawText(x, y, "0");

        # 横坐标 
        xStart = leftWidth
        yStart = yMax - downHeight
        painter.drawLine(xStart, yStart, xMax, yStart); 
        # 坐标短线
        length = xMax - xStart
        shortLineNums = int(length / xStep) - 1 # -1 预留单位text输出的位置
        shortLineLength = downHeight - xShortLineMagin
        if shortLineLength < 0:
            print("""短线边界过大,修改 = self.mConfig['短线边界']""")
            return
        for i in range(0, shortLineNums):
            x1 = (i + 1) * xStep + xStart
            x2 = x1
            y1 = yStart
            y2 = yStart + shortLineLength
            painter.drawLine(x1, y1, x2, y2 )
            text = "%.1f" % ((x1 - xStart) * self.mXPhyStep / 1000)
            #print(text)
            # +/-1 显示美观
            painter.drawText(x2 + 1, yMax - 1, text)
        painter.drawText(xMax - leftWidth, yMax - 1, xUnit)

        # 画纵坐标 
        xStart = leftWidth
        yStart = yMax - downHeight
        painter.drawLine(xStart, yStart, xStart, 0); 
        # 坐标短线
        length = yStart - 0 # 起点的y值为最大值
        shortLineNums = int(length / yStep) - 1 # -1 预留单位text输出的位置
        shortLineLength = leftWidth - yShortLineMagin
        if shortLineLength < 0:
            print("""短线边界过大,修改 = self.mConfig['短线边界']""")
            return
        # 计算纵向范围 
        yRange = shortLineNums * yStep
        # 每个像素代表的物理角度
        yPhyPerPix = self.mYPhyRange / yRange
        #print(yRange) 
        #print(yPhyPerPix) 
        for i in range(0, shortLineNums):
            x1 = xStart
            x2 = xStart - shortLineLength
            y1 = yStart - (i + 1) * yStep
            y2 = y1
            painter.drawLine(x1, y1, x2, y2)
            text = "%.1f" % ((((i + 1 ) * yStep) * yPhyPerPix) - (self.mYPhyRange / 2))
            #print(text)
            # +/-1 显示美观
            painter.drawText(0 + 1, y1 + 1, text)
        painter.drawText(0 + 1, 0 + downHeight, yUnit)

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

