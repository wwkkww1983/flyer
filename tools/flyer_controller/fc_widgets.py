#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt4 import QtGui
from PyQt4 import QtCore
from PyQt4 import QtOpenGL
from PyQt4.QtCore import pyqtSignal
from PyQt4.uic import loadUiType, loadUi

try:
    from OpenGL import GL
except ImportError: 
    app = QtGui.QApplication(sys.argv)
    QtGui.QMessageBox.critical(None, "飞控板控制",
            "需要安装PyOpenGL(使用pip installed PyOpenGL).\n")
    sys.exit(1) 
 
from fc_flyerWidget import FlyerWidget 

gGLSize = 680
FCWindowUIClass = loadUiType("fc_window.ui")

class FCWindow(QtGui.QWidget):
    def __init__(self):
        super(FCWindow, self).__init__() 
        
        # 初始化UI
        self.mUi = FCWindowUIClass[0]()
        self.mUi.setupUi(self)

        # 标题
        self.setWindowTitle("飞控板控制")

        # 绑定控制界面
        # 通信
        self.mComWidget = FCComWidget(self.mUi.typeComboBox, self.mUi.simIntervalLineEdit, self.mUi.comPortComboBox, self.mUi.buardrateLineEdit, self.mUi.ipLineEdit, self.mUi.ssidLineEdit, self.mUi.passwordLineEdit, self.mUi.usbNameComboBox, self.mUi.ctrlpushButton)
        # TODO:配置
        self.mCtrlWidget = FCCtrlWidget(self.mUi.xSlider, self.mUi.ySlider, self.mUi.zSlider, self.mUi.xLineEdit, self.mUi.yLineEdit, self.mUi.zLineEdit, self.mUi.lockPushButton)

        # TODO:姿态

        # 飞行器 加载OpenGL控件
        self.mFlyerWidget = FlyerWidget()
        self.mFlyerGroupBox = self.mUi.flyerGroupBox 
        self.mFlyerGroupBox.setFixedSize(gGLSize, gGLSize)
        vbox = QtGui.QVBoxLayout()
        vbox.addWidget(self.mFlyerWidget)
        self.mFlyerGroupBox.setLayout(vbox)

        # 绑定视角信号
        self.mCtrlWidget.sUpdateX.connect(self.mFlyerWidget.UpdateX)
        self.mCtrlWidget.sUpdateY.connect(self.mFlyerWidget.UpdateY)
        self.mCtrlWidget.sUpdateZ.connect(self.mFlyerWidget.UpdateZ)

        # 初始视角 经验值
        self.mCtrlWidget.UpdateX(0)
        self.mCtrlWidget.UpdateY(350)
        self.mCtrlWidget.UpdateZ(10)

class FCComWidget(QtGui.QWidget):
    def __init__(self, typeComboBox, simIntervalLineEdit, comPortComboBox, buardrateLineEdit, ipLineEdit, ssidLineEdit, passwordLineEdit, usbNameComboBox, ctrlpushButton):
        super(FCComWidget, self).__init__()

        self.mTypeComboBox = typeComboBox
        self.mSimIntervalLineEdit = simIntervalLineEdit
        self.mComPortComboBox = comPortComboBox
        self.mBuardrateLineEdit = buardrateLineEdit
        self.mIpLineEdit = ipLineEdit
        self.mSsidLineEdit = ssidLineEdit
        self.mPasswordLineEdit = passwordLineEdit
        self.mUsbNameComboBox = usbNameComboBox
        self.mCtrlpushButton = ctrlpushButton

        self.mTypeComboBox.addItem('模拟')
        self.mTypeComboBox.addItem('串口')
        self.mTypeComboBox.addItem('网络')
        self.mTypeComboBox.addItem('USB')

        self.mTypeComboBox.currentIndexChanged.connect(self.ChangeType) 
        
    def ChangeType(self, typeIndex): 
        if 0 != typeIndex: 
            msgBox = QtGui.QMessageBox();
            msgBox.setText("暂时只实现模拟数据.\n")
            msgBox.exec_();

            self.mTypeComboBox.setCurrentIndex(0)

class FCPostWidget(QtGui.QWidget):
    def __init__(self):
        super(FCPostWidget, self).__init__()

class FCCtrlWidget(QtGui.QWidget):
    sUpdateX = pyqtSignal(int, name='sUpdateX')
    sUpdateY = pyqtSignal(int, name='sUpdateY')
    sUpdateZ = pyqtSignal(int, name='sUpdateZ')

    def __init__(self, xSlider, ySlider, zSlider, xLineEdit, yLineEdit, zLineEdit, lockPushButton):
        super(FCCtrlWidget, self).__init__()
        self.mXSlider = xSlider
        self.mYSlider = ySlider 
        self.mZSlider = zSlider
        self.mXLineEdit = xLineEdit
        self.mYLineEdit = yLineEdit
        self.mZLineEdit = zLineEdit
        self.mLockPushButton = lockPushButton

        self.mXSlider.valueChanged.connect(self.UpdateX)
        self.mYSlider.valueChanged.connect(self.UpdateY)
        self.mZSlider.valueChanged.connect(self.UpdateZ)
        self.mXLineEdit.textChanged.connect(self.UpdateX)
        self.mYLineEdit.textChanged.connect(self.UpdateY)
        self.mZLineEdit.textChanged.connect(self.UpdateZ)

        self.mLocked = False
        self.mLockPushButton.clicked.connect(self.ChangeState)

    def ChangeState(self):
        if self.mLocked: 
            self.mXSlider.setEnabled(True)
            self.mYSlider.setEnabled(True)
            self.mZSlider.setEnabled(True)
            self.mXLineEdit.setEnabled(True)
            self.mYLineEdit.setEnabled(True)
            self.mZLineEdit.setEnabled(True)
            self.mLocked = False
            self.mLockPushButton.setText("锁定")
        else:
            self.mXSlider.setEnabled(False)
            self.mYSlider.setEnabled(False)
            self.mZSlider.setEnabled(False)
            self.mXLineEdit.setEnabled(False)
            self.mYLineEdit.setEnabled(False)
            self.mZLineEdit.setEnabled(False)
            self.mLocked = True
            self.mLockPushButton.setText("解锁")
        
    def UpdateX(self, angle):
        self.mXLineEdit.setText(str(angle))
        self.mXSlider.setValue(int(angle))
        self.sUpdateX.emit(int(angle))

    def UpdateY(self, angle):
        self.mYLineEdit.setText(str(angle))
        self.mYSlider.setValue(int(angle))
        self.sUpdateY.emit(int(angle))

    def UpdateZ(self, angle):
        self.mZLineEdit.setText(str(angle))
        self.mZSlider.setValue(int(angle))
        self.sUpdateZ.emit(int(angle))

if __name__ == '__main__': 
    app = QtGui.QApplication(sys.argv)
    win = FCWindow()
    win.show()
    sys.exit(app.exec_())

