#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.uic import loadUiType, loadUi

from fc_waveWidget import FCWaveWidget
from fc_frame import FCDataTimeAcceleratorDmpQuat

FCWindowUIClass = loadUiType("fc_analysisWidget.ui")

class FCAnalysisWidget(QWidget): 
    #sUpdateAcceleratorQuat = pyqtSignal((str, str, str, str, str, int, int, int, int, int), name='sUpdateAcceleratorQuat')

    def __init__(self):
        super(FCAnalysisWidget, self).__init__() 
        
        # 初始化UI
        self.mUi = FCWindowUIClass[0]()
        self.mUi.setupUi(self) 

        self.mPathLineEdit = self.mUi.pathLineEdit
        self.mBrowsePushButton = self.mUi.browsePushButton
        self.mDrawPushButton = self.mUi.drawPushButton
        
        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        self.mBrowsePushButton.clicked.connect(self.SetDataFilePath)
        self.mDrawPushButton.clicked.connect(self.Draw)

    def SetDataFilePath(self):
        print("未实现")

    def Draw(self): 
        data_file_path = self.mPathLineEdit.text()
        print("绘制以下文件中的数据:")
        print(data_file_path)

        data_file = open(data_file_path, 'rb')

        while True: 
            data = data_file.read(52)
            if not data:
                break
            frame = FCDataTimeAcceleratorDmpQuat(data) 
            #frame.Print()
            
            time = frame.GetTime()
            dmpQuat = frame.GetGmpQuat()
            print(time)
            print(dmpQuat.ToString())
            #euler = dmpQuat.ToEuler()
            #accelerator = frame.GetAccelrator() 
            
            #self.mWaveWidget.Append(time, euler, accelerator)

        data_file.close()
        print("绘制完成.")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCAnalysisWidget()
    win.show()
    sys.exit(app.exec_())

