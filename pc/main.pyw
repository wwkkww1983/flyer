#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""
os:win7 64
pyqt:PyQt5-5.6-gpl-Py3.5-Qt5.6.0-x64-2
opengl:pip install PyOpenGL(PyOpenGL3.1.0)
"""

import sys
from PyQt5.QtWidgets import QApplication
from PyQt5.QtWidgets import QMessageBox

from fc_window import FCWindow

try:
    from OpenGL import GL
except ImportError: 
    app = QApplication(sys.argv)
    QMessageBox.critical(None, "飞控上位机环境", 
    """
    需要以下环境
    os:win7 64
    pyqt:PyQt5-5.6-gpl-Py3.5-Qt5.6.0-x64-2
    opengl:pip install PyOpenGL(PyOpenGL3.1.0)
    """)
    sys.exit(1) 

if __name__ == '__main__': 
    app = QApplication(sys.argv)
    win = FCWindow()
    win.move(0,0)
    win.show()
    sys.exit(app.exec_())

