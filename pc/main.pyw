#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
os:win7 64
pyqt:PyQt5-5.6-gpl-Py3.5-Qt5.6.0-x64-2
"""
import sys
from PyQt5.QtWidgets import *

from widget import FCMainWindow

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCMainWindow()
    win.move(0,0)
    win.show()
    sys.exit(app.exec_())

