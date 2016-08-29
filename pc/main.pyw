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
    """
    from algo.quat import FCQuat
    gmpQuatTuplle = (0.1, -0.2, 0.3, 0.4)
    gmpQuat = FCQuat(*gmpQuatTuplle)
    gmpQuat.Print()

    euler = gmpQuat.ToEuler()
    euler.Print()
    """
    app = QApplication(sys.argv)
    win = FCMainWindow()
    win.move(0,0)
    #win.showFullScreen()
    win.show()
    sys.exit(app.exec_())

