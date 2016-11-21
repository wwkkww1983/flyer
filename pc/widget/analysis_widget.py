#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtGui import *
from PyQt5.QtCore import *
from PyQt5.QtWidgets import *

from PyQt5.uic import loadUiType, loadUi

from widget.wave_widget import FCWaveWidget
from frame.up import FCUpFrame

class FCAnalysisWidget(QWidget): 
    def __init__(self, uiFile):
        super(FCAnalysisWidget, self).__init__() 
        
        # 读取/设置ui文件
        UIClass = loadUiType(uiFile)
        self.mUi = UIClass[0]()
        self.mUi.setupUi(self) 

        # 初始化UI
        self.mPathLineEdit = self.mUi.pathLineEdit
        self.mBrowsePushButton = self.mUi.browsePushButton
        self.mDrawPushButton = self.mUi.drawPushButton
        self.mConsolePlainTextEdit = self.mUi.consolePlainTextEdit
        
        # 加入波形控件
        self.mWaveWidget = FCWaveWidget()
        self.mWaveGroupBox = self.mUi.waveGroupBox
        vbox = QVBoxLayout()
        vbox.addWidget(self.mWaveWidget)
        self.mWaveGroupBox.setLayout(vbox)

        self.mBrowsePushButton.clicked.connect(self.SetDataFilePath)
        self.mDrawPushButton.clicked.connect(self.Draw)

    def SetDataFilePath(self):
        data_file_path = QFileDialog.getOpenFileName()[0]
        self.mPathLineEdit.setText(data_file_path)

    def Draw(self): 
        data_file_path = self.mPathLineEdit.text()
        print("绘制以下文件中的数据:")
        print(data_file_path)
        data_file = open(data_file_path, 'rb')

        # FIXME:与frame_widget中_recv逻辑重复
        # 接收帧头  type + len = 8Bytes
        frameHeadLen = 8
        # 计算循环使用的常量
        while True:
            # 获取type+len
            frameHead = data_file.read(frameHeadLen)
            # 未获取到有效数据
            if not frameHead:
                break
            #print(frameHead)
            #FCUpFrame.PrintBytes(frameHead) 
            
            # 获取data+crc32
            frameDataAndCrc32Len = FCUpFrame.ParseLen(frameHead)
            #print(frameDataAndCrc32Len)
            #FCUpFrame.PrintBytes(frameHead)
            frameDataAndrCrc32 = data_file.read(frameDataAndCrc32Len)
            buf = frameHead + frameDataAndrCrc32 
            # 构造上行帧
            frame = FCUpFrame(buf) 
            (time, frameDict) = frame.ToFrameDict()
            
            # 文本帧
            if frameDict['文本']:
                text = '[%05d]:%s' % (time, frameDict['文本']) 
                # 等效于 append 但是不加入换行
                self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
                self.mConsolePlainTextEdit.insertPlainText(text)
                self.mConsolePlainTextEdit.moveCursor(QTextCursor.End)
            else: 
                self.mWaveWidget.Append(time, frameDict)

        self.mWaveWidget.update()
        data_file.close()
        print("绘制完成.")

if __name__ == '__main__':
    app = QApplication(sys.argv)
    win = FCAnalysisWidget()
    win.show()
    sys.exit(app.exec_())

