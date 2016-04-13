#!/usr/bin/env python3
# -*- coding: utf-8 -*-
from pyautocad import Autocad, APoint
acad = Autocad(create_if_not_exists = True)

def demo():
    # step 1:打开 autocad 2007
    # step 2:运行本脚本
    # 螺旋图
    print("文件名:") 
    print(acad.doc.Name) 
    p1 = APoint(0, 0)
    p2 = APoint(50, 25)
    acad.model.AddLine(p1, p2)
    acad.model.AddCircle(p1, 55)
    print("绘制完成.") 

if __name__ == '__main__': 
    demo()
