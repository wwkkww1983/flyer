#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import array
import json

from PyQt5 import QtGui
from fc_json_paser import FCJsonPaser
try:
    from OpenGL import GL
except ImportError: 
    app = QtGui.QApplication(sys.argv)
    QtGui.QMessageBox.critical(None, "飞控板控制",
            "需要安装PyOpenGL(使用pip installed PyOpenGL).\n")
    sys.exit(1) 

class FCAxis():
    def __init__(self, jsonName = 'axis.json'):
        self.mJsonPaser = FCJsonPaser(jsonName)
        self.mAxisScale = self.mJsonPaser.GetValue(("Configs", "Scale"))
        self.mArrowScale = self.mJsonPaser.GetValue(("Configs", "ArrowScale"))

        # X 北 红
        # X轴顶点 位置
        self.mXVertices = array.array('f',
                [0.0,               0.0,             -1.0,                    self.mAxisScale,
                 0.0,               0.0,              1.0,                    self.mAxisScale,
                 self.mArrowScale,  0.0,              1.0 - self.mArrowScale, self.mAxisScale,
                 -self.mArrowScale, 0.0,              1.0 - self.mArrowScale, self.mAxisScale,
                 0.0,               self.mArrowScale, 1.0 - self.mArrowScale, self.mAxisScale,
                 0.0,              -self.mArrowScale, 1.0 - self.mArrowScale, self.mAxisScale])
        # X轴顶点 颜色
        self.mXColors = array.array('f',
                [ 0.3,  0.0,  0.0,  1.0,
                  0.3,  0.0,  0.0,  1.0,
                  0.3,  0.0,  0.0,  1.0,
                  0.3,  0.0,  0.0,  1.0,
                  0.3,  0.0,  0.0,  1.0,
                  0.3,  0.0,  0.0,  1.0] ) 
        # X轴顶点索引
        self.mXIndices = [
                array.array('B', [0, 1]),
                array.array('B', [1, 2]),
                array.array('B', [1, 3]),
                array.array('B', [1, 4]),
                array.array('B', [1, 5])]

        # Y 东 绿
        # Y轴顶点 位置
        self.mYVertices = array.array('f',
                [-1.0,                    0.0,               0.0,               self.mAxisScale,
                  1.0,                    0.0,               0.0,               self.mAxisScale,
                  1.0 - self.mArrowScale, self.mArrowScale,  0.0,               self.mAxisScale,
                  1.0 - self.mArrowScale, -self.mArrowScale, 0.0,               self.mAxisScale,
                  1.0 - self.mArrowScale, 0.0,               self.mArrowScale,  self.mAxisScale,
                  1.0 - self.mArrowScale, 0.0,               -self.mArrowScale, self.mAxisScale]) 
        # Y轴顶点 颜色
        self.mYColors = array.array('f',
                [ 0.0,  0.2,  0.0,  1.0,
                  0.0,  0.2,  0.0,  1.0,
                  0.0,  0.2,  0.0,  1.0,
                  0.0,  0.2,  0.0,  1.0,
                  0.0,  0.2,  0.0,  1.0,
                  0.0,  0.2,  0.0,  1.0] ) 
        # Y轴顶点索引
        self.mYIndices = [
                array.array('B', [0, 1]),
                array.array('B', [1, 2]),
                array.array('B', [1, 3]),
                array.array('B', [1, 4]),
                array.array('B', [1, 5])]

        # Z 天 蓝
        # Z轴顶点 位置
        self.mZVertices = array.array('f',
                [0.0,                1.0,                    0.0,              self.mAxisScale,
                 0.0,               -1.0,                    0.0,              self.mAxisScale,
                 self.mArrowScale,  -1.0 + self.mArrowScale, 0.0,              self.mAxisScale,
                 -self.mArrowScale, -1.0 + self.mArrowScale, 0.0,              self.mAxisScale,
                 0.0,               -1.0 + self.mArrowScale, self.mArrowScale, self.mAxisScale,
                 0.0,               -1.0 + self.mArrowScale, -self.mArrowScale, self.mAxisScale])
        # Z轴顶点 颜色
        self.mZColors = array.array('f',
                [ 0.0,  0.0,  0.4,  1.0,
                  0.0,  0.0,  0.4,  1.0,
                  0.0,  0.0,  0.4,  1.0,
                  0.0,  0.0,  0.4,  1.0,
                  0.0,  0.0,  0.4,  1.0,
                  0.0,  0.0,  0.4,  1.0] ) 
        # Z轴顶点索引
        self.mZIndices = [
                array.array('B', [0, 1]),
                array.array('B', [1, 2]),
                array.array('B', [1, 3]),
                array.array('B', [1, 4]),
                array.array('B', [1, 5])]


        # 三轴汇总

    def PaintGL(self):
        for axis in ('x', 'y', 'z'): 
            GL.glEnableClientState( GL.GL_COLOR_ARRAY )
            GL.glEnableClientState( GL.GL_VERTEX_ARRAY )
            GL.glColorPointer(4, GL.GL_FLOAT, 0, self.GetColors(axis))
            GL.glVertexPointer(4, GL.GL_FLOAT, 0, self.GetVertices(axis)) 
            GL.glLineWidth(1)
            for i in range(0, 5):
                GL.glDrawElements(GL.GL_LINES, 2, GL.GL_UNSIGNED_BYTE, self.GetIndices(axis, i))
            GL.glDisableClientState( GL.GL_COLOR_ARRAY )
            GL.glDisableClientState( GL.GL_VERTEX_ARRAY ) 

    def GetVertices(self, axisName):
        if 'x' == axisName:
            return self.mXVertices.tostring()
        elif 'y' == axisName:
            return self.mYVertices.tostring()
        elif 'z' == axisName:
            return self.mZVertices.tostring()
        else:
            print('%s 不是坐标轴' % axisName)

    def GetColors(self, axisName):
        if 'x' == axisName:
            return self.mXColors.tostring()
        elif 'y' == axisName:
            return self.mYColors.tostring()
        elif 'z' == axisName:
            return self.mZColors.tostring()
        else:
            print('%s 不是坐标轴' % axisName)

    def GetIndices(self, axisName, index):
        if 'x' == axisName:
            return self.mXIndices[index].tostring()
        elif 'y' == axisName:
            return self.mYIndices[index].tostring()
        elif 'z' == axisName:
            return self.mZIndices[index].tostring()
        else:
            print('%s 不是坐标轴' % axisName)

if __name__ == '__main__': 
    axis = FCAxis()
    axis.GetVertices('x')
    axis.GetColors('x')
    axis.GetIndices('x', 0)
    print('测试通过')

