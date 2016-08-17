#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys

from PyQt5.QtWidgets import *
from PyQt5.QtCore import pyqtSignal
from PyQt5.uic import loadUiType, loadUi

from fc_widget import FCWidget

FCWindowUIClass = loadUiType("fc_window.ui")

class FCWindow(QMainWindow):
    def __init__(self):
        super(FCWindow, self).__init__() 

        # 初始化UI
        self.mUi = FCWindowUIClass[0]()
        self.mUi.setupUi(self)

        # 菜单在ui文件中初始化
        # 此处指初始化状态栏
        statusLabel1 = QLabel("状态栏占位标签")
        self.mStatusBar = self.mUi.statusbar
        self.mStatusBar.addWidget(statusLabel1)

        # 标题
        self.setWindowTitle("飞控上位机") 
        
        # 设置核心控件
        self.mFCWidget = FCWidget()
        self.setCentralWidget(self.mFCWidget)

    def closeEvent(self, event):
        self.mFCWidget.close()

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCWindow()
    win.show()
    sys.exit(app.exec_())

