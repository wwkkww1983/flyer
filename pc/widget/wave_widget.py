#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from config import gWaveXUnit, gWaveXStep, gWaveYUint, gWaveYStep, gWaveAxesColor, gWaveAxesWidth, gXTimePerPixelRate, gXPixelPerTimemsRate, gYAnglePerPixelRate, gYPixelPerAngleRate, gWaveConfig

from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.QtGui import *

class FCWaveWidget(QWidget):
    # msMove = pyqtSignal(QPoint, name='msMove')
    msMove = pyqtSignal(['QPoint', 'int'], name = "msMove")

    def __init__(self):
        super(FCWaveWidget, self).__init__() 

        # 存放 下位机采样的数据
        self.mData = []

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

        # 使用列表相较于字典有序
        aData = (time, frameDict)
        self.mData.append(aData)

    def paintEvent(self, paintEvent):
        painter = QPainter(self) 

        # 清屏
        painter.fillRect(0, 0, self.width(), self.height(), Qt.black)

        # 更新绘制参数(生成绘布范围,故必须最先调用)
        self.updateParas(painter)

        # 绘制坐标系
        self.drawAxes(painter)

        # 绘制图例
        self.drawIcon(painter)

        # 绘制波形 
        self.drawWave(painter)

        # 绘制鼠标位置十字线
        self.drawMouseCross(painter)

        painter.end() 

    def updateParas(self, painter):
        # 可绘制区域需要扣除label区域
        self.mXMax = self.width()
        self.mYMax = self.height()
        metrics = painter.fontMetrics()
        self.mLeftWidth = metrics.width(gWaveYUint)
        self.mDownHeigt = metrics.ascent() + metrics.descent()

        # 可绘制区坐标原点(屏幕坐标原点在左上,几何坐标原点在左下)
        # 可绘制区域左下点坐标(self.mXDrawOrig, self.mYDrawOrig)
        # 可绘制区域右上点坐标(self.mXMax, 0)
        self.mXDrawOrig = self.mLeftWidth
        self.mYDrawOrig = self.mYMax - self.mDownHeigt
        self.mXDrawLength = self.mXMax - self.mLeftWidth
        self.mYDrawLength = self.mYMax - self.mDownHeigt
        self.mXDrawEnd = self.mXMax
        self.mYDrawEnd = 0

        self.mXDataOrig = int(self.mXDrawOrig)
        self.mYDataOrig = int(self.mYDrawOrig / 2)

    def drawAxes(self, painter):
        pen = QPen(gWaveAxesColor)
        pen.setWidth(gWaveAxesWidth)
        painter.setPen(pen)

        # x/y坐标
        painter.drawLine(self.mXDrawOrig, self.mYDrawOrig, self.mXDrawEnd, self.mYDrawOrig)
        painter.drawLine(self.mXDrawOrig, self.mYDrawOrig, self.mXDrawOrig, self.mYDrawEnd)

        # x 标签
        for x in range(self.mXDrawOrig, self.mXDrawEnd, gWaveXStep):
            painter.drawLine(x, self.mYDrawOrig, x, self.mYDrawEnd)

            text = "%+.1f" % ((x - self.mXDrawOrig) * gXTimePerPixelRate)
            #print(text)
            # +/-1 显示美观
            painter.drawText(x + 1, self.mYMax - 1, text)

        # y标签 > 0
        for y in range(self.mYDataOrig, self.mYDrawEnd, -gWaveYStep):
            painter.drawLine(self.mXDrawOrig, y, self.mXDrawEnd, y)

            text = "%+.1f" % ((self.mYDrawOrig - self.mYDataOrig - y) * gYAnglePerPixelRate)
            #print(text)
            # +/-1 显示美观
            painter.drawText(0 + 1, y, text)

        # y标签 < 0
        for y in range(self.mYDataOrig + gWaveYStep, self.mYDrawOrig, gWaveYStep):
            painter.drawLine(self.mXDrawOrig, y, self.mXDrawEnd, y)

            text = "%+.1f" % ((self.mYDrawOrig - self.mYDataOrig - y) * gYAnglePerPixelRate)
            #print(text)
            # +/-1 显示美观
            painter.drawText(0 + 1, y, text)

        # 绘制过原点平行于x轴的直线
        pen = QPen(Qt.white)
        pen.setWidth(1)
        painter.setPen(pen)
        painter.drawLine(self.mXDataOrig, self.mYDataOrig, self.mXDrawEnd, self.mYDataOrig)

    def drawIcon(self, painter):
        # 找出键值字符串最长的数据项
        maxNameLen = 0
        metrics = painter.fontMetrics()
        for aConfig in gWaveConfig:
            name = aConfig[0]
            nowNameLen = metrics.width(name)
            if maxNameLen < nowNameLen:
                maxNameLen = nowNameLen
        # print(maxNameLen)

        # 图例行距
        adj = 5 # 用于美观的调整
        yStep = metrics.ascent() + metrics.descent() + adj

        # 右上角开始绘制图例
        xStart = self.mXMax - maxNameLen - adj
        yStart = adj
        for aConfig in gWaveConfig:
            name = aConfig[0]
            color = aConfig[1]
            if None == color: # None表示不绘制
                continue
            width = aConfig[2]

            pen = QPen(color)
            pen.setWidth(width)
            painter.setPen(pen)
            yStart += yStep
            painter.drawText(xStart, yStart, name)

    def drawWave(self, painter):
        # 绘制波形
        self.drawEulerTheta(painter)

    def drawEulerTheta(self, painter):
        # 没有数据 不绘制
        allData = self.mData
        if [] == allData:
            return

        # 绘制起点
        xOrig = allData[0][0]
        xPhyStart = None
        yPhyStart = None

        # 绘制参数
        dataName = '俯仰角'
        color = None
        width = None
        for aConfig in gWaveConfig:
            if dataName == aConfig[0]:
                color = aConfig[1]
                width = aConfig[2]

        # 不可绘制
        if None == color or None == width:
            return

        pen = QPen(color)
        pen.setWidth(width)
        painter.setPen(pen)

        # 遍历绘制 
        for aData in allData:
            # 当前数据与起点的时间差(物理x)
            xPhy = aData[0]

            # 俯仰角值(物理y)
            frameDict = aData[1]
            euler = aData[1]['欧拉角']
            if None == euler: # 该数据没有欧拉角值
                continue

            theta = euler.Theta()
            if (None == xPhyStart) or (None == yPhyStart): # 起始点
                xPhyStart = xPhy
                yPhyStart = theta
            else: # 实际绘制
                yPhy = theta

                # 物理坐标到绘制坐标
                xScreenStart = int((xPhyStart - xOrig) * gXPixelPerTimemsRate)
                xScreenEnd = int((xPhy - xOrig) * gXPixelPerTimemsRate)
                yScreenStart = int(yPhyStart * gYPixelPerAngleRate)
                yScreenEnd = int(yPhy * gYPixelPerAngleRate)
                # 绘制
                painter.drawLine(xScreenStart, yScreenStart, xScreenEnd, yScreenEnd)

                print(xScreenStart, yScreenStart, xScreenEnd, yScreenEnd)

                # 为下一数据绘制准备
                xPhyStart = xPhy
                yPhyStart = yPhy


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

