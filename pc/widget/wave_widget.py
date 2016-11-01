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

        # 配置
        self.mConfig = {
                '横坐标单位' : '秒',
                '横坐标步长' : 50,

                '纵坐标单位' : '  度  ',
                '纵坐标步长' : 30,
                }
        # 坐标颜色(字典无顺序,使用列表)
        self.mColor = [
                ('俯仰角', Qt.red),
                #('横滚角', Qt.green),
                #('偏航角', Qt.blue),
                ('前油门',Qt.blue,),
                ('后油门', Qt.yellow),
                #('左油门', Qt.cyan),
                #('右油门', Qt.magenta),
                ]
        
        # TODO: 使用参数传入
        self.mXPhyStep = 100  # 每个像素点为100ms间隔
        self.mYAngleRange = 150 # 纵坐标角度范围 120deg
        self.mYAnglePerPix = 0
        self.mYPidRange = 1000 # 纵坐标Pid范围 1000
        self.mYPidPerPix = 0
        self.mXOrig = 0
        self.mYOrig = 0
        self.mWaveWidth = 0
        self.mWaveHeight = 0

        # 存放 下位机采样的数据
        self.mData = []

        self.setMouseTracking(True) # 鼠标跟踪
        self.setCursor(Qt.BlankCursor) # 隐藏鼠标
        self.mPos = None # 鼠标当前点

    def Append(self, frame):
        self.mData.append(frame)

    def paintEvent(self, paintEvent):
        painter = QPainter(self) 

        # 清屏
        painter.fillRect(0, 0, self.width(), self.height(), Qt.black)
        # 绘制坐标系(生成绘布范围,故必须最先调用)
        self.drawAxes(painter) 
        # 绘制图例
        #self.drawIcon(painter)
        # 绘制波形 
        self.drawWaveEulerTheta(painter)
        self.drawWavePidTheta(painter)
        #self.drawWave(painter)
        # 绘制鼠标位置十字线
        self.drawMouseCross(painter)

        painter.end() 

    def drawAxes(self, painter): 
        xMax = self.width()
        yMax = self.height()

        metrics = painter.fontMetrics() 
        xUnit = self.mConfig['横坐标单位']
        xStep = self.mConfig['横坐标步长']
        yUnit = self.mConfig['纵坐标单位']
        yStep = self.mConfig['纵坐标步长']

        #print(xUnit)
        #print(yUnit)
        leftWidth = metrics.width(yUnit)
        downHeight = metrics.ascent() + metrics.descent()
        #print(leftWidth)
        #print(downHeight)

        # 绘制坐标系
        pen = QPen(Qt.darkGray)
        pen.setWidth(1)
        painter.setPen(pen)

        xStart = leftWidth
        yStart = yMax - downHeight

        # 横坐标 
        painter.drawLine(xStart, yStart, xMax, yStart); 
        # 坐标短线
        length = xMax - xStart
        shortLineNums = int(length / xStep)
        for i in range(0, shortLineNums):
            x1 = i * xStep + xStart
            x2 = x1
            y1 = yStart
            y2 = 0
            painter.drawLine(x1, y1, x2, 0)
            text = "%+.1f" % ((x1 - xStart) * self.mXPhyStep / 1000)
            #print(text)
            # +/-1 显示美观
            painter.drawText(x2 + 1, yMax - 1, text)
        painter.drawText(xMax - leftWidth, yMax - 1, xUnit)

        # 画纵坐标 
        painter.drawLine(xStart, yStart, xStart, 0); 
        # 坐标短线
        length = yStart - 0 # 起点的y值为最大值
        shortLineNums = int(length / yStep)
        # 计算纵向范围 
        yRange = shortLineNums * yStep
        # 每个像素代表的物理角度
        self.mYAnglePerPix = self.mYAngleRange / yRange
        #print(yRange) 
        #print(self.mYAnglePerPix) 
        for i in range(0, shortLineNums):
            x1 = xStart
            x2 = xMax
            y1 = yStart - i * yStep
            y2 = y1
            painter.drawLine(x1, y1, x2, y2)
            text = "%+.2f" % ((i * yStep * self.mYAnglePerPix) - (self.mYAngleRange / 2))
            #print(text)
            # +/-1 显示美观
            painter.drawText(0 + 1, y1 + 1, text)
        painter.drawText(0 + 1, 0 + downHeight, yUnit)

        # 坐标原点
        xOrig = xStart
        yOrig = yStart - yRange / 2
        # 绘制过原点平行于x轴的直线 
        pen = QPen(Qt.white)
        pen.setWidth(1)
        painter.setPen(pen)
        painter.drawLine(xOrig, yOrig, xMax, yOrig)
        # 标示原点比较不协调
        #painter.drawText(xOrig + 1, yOrig - 1, "0");

        # 画布原点
        self.mXOrig = xOrig
        self.mYOrig = yOrig
        # 画布尺寸
        self.mWaveWidth = xMax
        self.mWaveHeight = yStart

        # 每个像素代表的物理角度
        self.mYPidPerPix = self.mYPidRange / self.mWaveWidth

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

    def _drawAIcon(self, painter, x, y, text, color):
        pen = QPen(color)
        painter.setPen(pen) 
        metrics = painter.fontMetrics() 
        textWidth = metrics.width(text)
        textHeight = metrics.ascent() + metrics.descent()
        painter.drawText(x - textWidth, y + textHeight, text)

    def drawIcon(self, painter):
        metrics = painter.fontMetrics() 
        yStep = metrics.ascent() + metrics.descent() + 10
        x = self.mWaveWidth
        i = 0 
        for val in self.mColor: 
            y = i * yStep
            text = val[0]
            color = val[1]
            i = i + 1 
            self._drawAIcon(painter, x, y, text, color)

    def drawWaveEulerTheta(self, painter): 
        # 无数据不绘制
        if not self.mData:
            return
        
        firstFrame = self.mData[0]
        startTime  = firstFrame.GetTime() / 100 # 转换为秒为单位

        length = len(self.mData)
        for i in range(0, length):
            #thetaText = "俯仰角:%+04.2fd" % euler.Theta()
            #phiText   = "横滚角:%+04.2fd" % euler.Phi()
            #psiText   = "偏航角:%+04.2fd" % euler.Psi()

            # 首尾不绘制
            if 0 == i:
                continue
            if length - 1 == i:
                break

            lastFrame = self.mData[i - 1]
            lastTime  = lastFrame.GetTime() / 100
            lastEuler = lastFrame.GetEuler()

            nowFrame  = self.mData[i]
            nowTime   = nowFrame.GetTime() / 100
            nowEuler  = nowFrame.GetEuler()

            # 俯仰角
            xLast = lastTime - startTime + self.mXOrig
            xNow = nowTime - startTime + self.mXOrig
            yThetaLast = self.mYOrig - lastEuler[0] / self.mYAnglePerPix
            yThetaNow = self.mYOrig - nowEuler[0] / self.mYAnglePerPix
            pen = QPen(Qt.red)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yThetaLast, xNow, yThetaNow)

    def drawWavePidTheta(self, painter): 
        # 无数据不绘制
        if not self.mData:
            return
        
        firstFrame = self.mData[0]
        startTime  = firstFrame.GetTime() / 100 # 转换为秒为单位

        length = len(self.mData)
        for i in range(0, length):
            #thetaText = "俯仰角:%+04.2fd" % euler.Theta()
            #phiText   = "横滚角:%+04.2fd" % euler.Phi()
            #psiText   = "偏航角:%+04.2fd" % euler.Psi()

            # 首尾不绘制
            if 0 == i:
                continue
            if length - 1 == i:
                break

            lastFrame = self.mData[i - 1]
            lastTime  = lastFrame.GetTime() / 100
            lastPid = lastFrame.GetPid()

            nowFrame  = self.mData[i]
            nowTime   = nowFrame.GetTime() / 100
            nowPid    = nowFrame.GetPid()

            # 俯仰角
            xLast = lastTime - startTime + self.mXOrig
            xNow = nowTime - startTime + self.mXOrig
            yPidLast = self.mYOrig - lastPid[0] / self.mYPidPerPix
            yPidNow = self.mYOrig - nowPid[0] / self.mYPidPerPix
            pen = QPen(Qt.green)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yPidLast, xNow, yPidNow)

    def drawWave(self, painter): 
        # 无数据不绘制
        if not self.mData:
            return
        
        firstFrame = self.mData[0]
        timeStart = firstFrame.GetTime() / 100 # 转换为秒为单位

        length = len(self.mData)
        for i in range(0, length):
            #thetaText = "俯仰角:%+04.2fd" % euler.Theta()
            #phiText   = "横滚角:%+04.2fd" % euler.Phi()
            #psiText   = "偏航角:%+04.2fd" % euler.Psi()

            # 首尾不绘制
            if 0 == i:
                continue
            if length - 1 == i:
                break

            dataLast = self.mData[i - 1]
            timeLast = dataLast[0] / 100
            eulerLast = dataLast[1]
            acceleratorLast = dataLast[2]

            dataNow = self.mData[i]
            timeNow = dataNow[0] / 100
            eulerNow = dataNow[1]
            acceleratorNow = dataNow[2]

            acceleratorMax = acceleratorLast[4]
            pixelPerAccelerator = self.mWaveHeight / acceleratorMax

            # 转换为绘制坐标
            xLast = timeLast - timeStart + self.mXOrig
            yThetaLast = self.mYOrig - eulerLast.Theta() / self.mYAnglePerPix
            yPhiLast = self.mYOrig - eulerLast.Phi() / self.mYAnglePerPix
            yPsiLast = self.mYOrig - eulerLast.Psi() / self.mYAnglePerPix
            yFrontLast = self.mWaveHeight - acceleratorLast[0] * pixelPerAccelerator
            yRightLast = self.mWaveHeight - acceleratorLast[1] * pixelPerAccelerator
            yBackLast  = self.mWaveHeight - acceleratorLast[2] * pixelPerAccelerator
            yLeftLast  = self.mWaveHeight - acceleratorLast[3] * pixelPerAccelerator

            xNow = timeNow - timeStart + self.mXOrig
            yThetaNow = self.mYOrig - eulerNow.Theta() / self.mYAnglePerPix
            yPhiNow = self.mYOrig - eulerNow.Phi() / self.mYAnglePerPix
            yPsiNow = self.mYOrig - eulerNow.Psi() / self.mYAnglePerPix
            yFrontNow = self.mWaveHeight - acceleratorNow[0] * pixelPerAccelerator
            yRightNow = self.mWaveHeight - acceleratorNow[1] * pixelPerAccelerator
            yBackLNow = self.mWaveHeight - acceleratorNow[2] * pixelPerAccelerator
            yLeftLNow = self.mWaveHeight - acceleratorNow[3] * pixelPerAccelerator

            # theta 俯仰角
            pen = QPen(Qt.red)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yThetaLast, xNow, yThetaNow)

            """
            # phi 横滚角度
            pen = QPen(Qt.green)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yPhiLast, xNow, yPhiNow)
            # psi 偏航角度
            pen = QPen(Qt.blue)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yPsiLast, xNow, yPsiNow)
            """
            # 前油门
            pen = QPen(Qt.blue)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yFrontLast, xNow, yFrontNow)

            """
            # 后油门
            pen = QPen(Qt.yellow)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yBackLast, xNow, yBackLNow)

            print("begin")
            print((xLast, yThetaLast), (xNow, yThetaNow))
            print((xLast, yFrontLast), (xNow, yFrontNow))
            print((xLast, yBackLast), (xNow, yBackLNow))
            print("end")

            # 左油门
            pen = QPen(Qt.cyan)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yLeftLast, xNow, yLeftLNow)

            # 右油门
            pen = QPen(Qt.magenta)
            pen.setWidth(1)
            painter.setPen(pen) 
            painter.drawLine(xLast, yRightLast, xNow, yRightNow)
            """

            #print(time)
            #print((yTheta, yPhi, yPsi))

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

