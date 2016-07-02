#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5 import QtGui
from PyQt5 import QtCore
from PyQt5 import QtWidgets

from fc_axis import FCAxis
from fc_flyer import FCFlyer

try:
    from OpenGL import GL
    from OpenGL import GLU
except ImportError: 
    app = QtGui.QApplication(sys.argv)
    QtGui.QMessageBox.critical(None, "飞控板控制",
            "需要安装PyOpenGL(使用pip installed PyOpenGL).\n")
    sys.exit(1) 
 
class FlyerWidget(QtWidgets.QOpenGLWidget):
    def __init__(self, parent=None):
        super(FlyerWidget, self).__init__(parent)

        # 三维物体 及其初始角度
        self.mAxis = FCAxis() 
        self.mFlyer = FCFlyer()

        # 拖拽起点
        self.mDragStart = None
        self.mXRot = 0
        self.mYRot = 0
        self.mZRot = 0
        
        # 绘制定时器
        self.mPaintTimer = None
        self.mFrameCount = 1

    def initializeGL(self):
        #self.PrintGLInfo()
        GL.glClearColor(0.0, 0.0, 0.0, 1.0)
        GL.glEnable(GL.GL_DEPTH_TEST | GL.GL_LINE_SMOOTH)

        self.mPaintTimer = QtCore.QTimer();
        self.mPaintTimer.timeout.connect(self.update);
        #self.mPaintTimer.start(500) # 初始时以不可达的1000fps刷新

    def UpdateX(self, xRot):
        self.mXRot = xRot
        self.update()

    def UpdateY(self, yRot):
        self.mYRot = yRot
        self.update()

    def UpdateZ(self, zRot):
        self.mZRot = zRot
        self.update()

    def paintGL(self):
        GL.glClear(GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT ) 
        GL.glViewport(0, 0, self.width(), self.height()) 

        GL.glLoadIdentity()

        # 旋转
        GL.glRotated(self.mXRot, 0.0, 0.0, 1.0) # 北 opengl z
        GL.glRotated(self.mYRot, 1.0, 0.0, 0.0) # 东 opengl x
        GL.glRotated(self.mZRot, 0.0, 1.0, 0.0) # 天 opengl y

        self.mAxis.PaintGL()
        self.mFlyer.PaintGL()
        #GL.glFinish()

        #print("frame:%d" % self.mFrameCount, end='')
        #print("xRot:%f," % self.mXRot, end='')
        #print("yRot:%f," % self.mYRot, end='')
        #print("zRot:%f." % self.mZRot)
        self.mFrameCount += 1

    def PrintGLInfo(self):
        print("生产商       ", end=":")
        print(GL.glGetString(GL.GL_VENDOR))

        print("渲染器       ", end=":")
        print(GL.glGetString(GL.GL_RENDERER))

        print("OpenGL版本   ", end=":")
        print(GL.glGetString(GL.GL_VERSION))

        print("着色语言版本 ", end=":")
        print(GL.glGetString(GL.GL_SHADING_LANGUAGE_VERSION))

        """
        print("API:")
        for func in dir(GL):
            print(func) 
        print(GL.__doc__)
        """

    # 交互类函数
    def mousePressEvent(self, mouseEvent):
        pos = mouseEvent.pos()
        self.mDragStart = pos
        #print("开始拖拽:(%d,%d)" % (pos.x(), pos.y()))

    def mouseReleaseEvent (self, mouseEvent):
        pos = mouseEvent.pos()
        x = pos.x() - self.mDragStart.x()
        y = pos.y() - self.mDragStart.y()

        """
        if 0 == x and 0 == y:
            print('没有拖拽')
            return

        if 0 != x: 
            print('左右移动:%d' % x) 

        if 0 != y:
            print('上下移动:%d' % y)
        """

if __name__ == '__main__': 
    app = QtGui.QApplication(sys.argv)
    win = FlyerWidget()
    win.show()
    win.setFixedSize(500,500)
    sys.exit(app.exec_())

