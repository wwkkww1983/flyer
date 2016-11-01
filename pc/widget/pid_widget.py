#!/usr/bin/env python3
# -*- coding: utf-8 -*-


from config import gLocalIP
from config import gLocalPort
from config import gSaveDataFileFullName

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *
from PyQt5.uic import loadUiType, loadUi 

from widget.wave_widget import FCWaveWidget
from widget.frame_widget import FCFrameWidget

class FCPidWidget(FCFrameWidget): 
    def __init__(self, uiFile):
        print(uiFile)
        super(FCPidWidget, self).__init__() 

        # 读取/设置ui文件
        UIClass = loadUiType(uiFile)
        self.mUi = UIClass[0]()
        self.mUi.setupUi(self) 

        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        # 基本配置
        self.mIpLabel = self.mUi.ipLabel
        self.mPortLabel = self.mUi.portLabel
        self.mDataPathLabel = self.mUi.dataPathLabel
        self.mIpLabel.setText("IP:" + str(gLocalIP))
        self.mPortLabel.setText("Port:" + str(gLocalPort))
        self.mDataPathLabel.setText("信息保存到:" + str(gSaveDataFileFullName))

        # 控制台文本输出
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit

    def closeEvent(self, event):
        super(FCPidWidget, self).closeEvent(event)

    def RecvNewUpFrame(self, frame): 
        """
        接收新帧槽
        """
        #print("FCPidWidget.RecvNewUpFrame") 
        (time, frameDict) = frame.ToFrameDict() 
        # print(time)
        # frame.PrintDict()

        # 文本帧
        if frameDict['text']:
            text = '[%05d]:%s' % (time, frameDict['text'])

            # 等效于 append 但是不加入换行
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
            self.mConsolePlainTextEdit.insertPlainText(text)
            self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
        else: 
            self.mWaveWidget.Append(frame)

        #print()

