#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
os:win7 64
pyqt:PyQt5-5.6-gpl-Py3.5-Qt5.6.0-x64-2
opengl:pip install PyOpenGL(PyOpenGL3.1.0)
"""

import sys
from PyQt5 import QtWidgets

from fc_widgets import FCWindow

app = QtWidgets.QApplication(sys.argv)
win= FCWindow()
win.move(0,0)
win.show()
sys.exit(app.exec_())


