#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import math

g_config = {}

# 马达参数
g_config["motor_hole_out_r"] = 6.0

# 可配置参数 标准大小 以150mm板子来做
g_config["paper_uint"] = "Millimeter"
g_config["paper_precision"] = 0.01
g_config["paper_margin"] = 5.0
g_config["board_size"] = 150.0

# 主信号区域
g_config["signal_area_size"] = 70.0

# 图纸相关
g_config["paper_size"] = g_config["board_size"] + g_config["paper_margin"] * 2
g_config["paper_x0"] = -g_config["paper_size"]/2
g_config["paper_y0"] = g_config["paper_x0"]

# 悬链线系数
g_catenary_a = 0.0

# 初始化板参数
g_scr = """# Allegro script
# mm为单位
# paper_uint %s
# paper_precision %.4f
# paper_margin %.4f
# board_size %.4f

version 16.5

setwindow pcb
trapsize 1362
generaledit
new 
newdrawfillin "flyer.dra" "Mechanical Symbol"
trapsize 1362
trapsize 1362
generaledit 

pop dyn_option_select 'Quick Utilities@:@Design Parameters...' 
prmed 
setwindow form.prmedit
FORM prmedit design  
FORM prmedit units %s
FORM prmedit x %.4f
FORM prmedit y %.4f
FORM prmedit width %.4f
FORM prmedit height %.4f
FORM prmedit done  
trapsize 12821
generaledit 
setwindow pcb

# 精度为0.01mm
pop dyn_option_select 'Quick Utilities@:@Grids...' 
define grid 
generaledit 
setwindow form.grid
FORM grid non_etch non_etch_x_grids %.4f
FORM grid non_etch non_etch_y_grids %.4f
FORM grid top subclass_x_grids %.4f
FORM grid top subclass_y_grids %.4f
FORM grid bottom subclass_x_grids %.4f
FORM grid bottom subclass_y_grids %.4f
FORM grid bottom subclass_x_grids %.4f
FORM grid display YES 
FORM grid done  
setwindow pcb

""" %  ( g_config["paper_uint"], g_config["paper_precision"], g_config["paper_margin"], g_config["board_size"], 
        g_config["paper_uint"], 
        g_config["paper_x0"], g_config["paper_y0"], g_config["paper_size"], g_config["paper_size"], 
        g_config["paper_precision"], g_config["paper_precision"], g_config["paper_precision"], g_config["paper_precision"], g_config["paper_precision"], g_config["paper_precision"], g_config["paper_precision"])

def get_up_outline():
    """
    生成正上方outline,其他三个(左,下,右)旋转获得
    """
    # 精度
    precision = g_config["paper_precision"]
    # 半径
    circle_r = float(g_config["motor_hole_out_r"])

    # 马达孔的外包圆弧1 3点法表示
    circle_list1 = []
    # -circle_r to (circle_r * sin(pi/4))
    x_min = -circle_r
    x_max = circle_r * math.sin(math.pi/4)
    #print(x_min, x_max)
    x = x_min - precision
    while x < x_max:
        x = x + precision
        y = math.sqrt(math.pow(circle_r, 2) - math.pow(x , 2)) 
        circle_list1.append([x, y])
    #print(circle_list1[0])
    # 平移到右上角 
    x_tran = (g_config["board_size"] / 2) - g_config["motor_hole_out_r"]
    y_tran = x_tran
    for p in circle_list1:
        p[0] = p[0] + x_tran
        p[1] = p[1] + y_tran
    #print(circle_list1[0])

    # 马达孔的外包圆弧2 由圆弧1关于y轴对称获得
    circle_list2 = []
    for p in circle_list1:
        circle_list2.append([-p[0], p[1]])

    # 悬链线
    catenary_list = []
    # x范围
    x_max = (g_config["board_size"] / 2) - (g_config["motor_hole_out_r"] * 2)
    y_max = x_max - (g_config["signal_area_size"] /2)
    x_min = -x_max
    x_max = x_max
    #print("%.4f, %.4f, %.4f" % (x_min, x_max, y_max)) 
    #print("%.5f" % g_catenary_a) 

    x = x_min - precision
    # - 1e-10 避免溢出
    while x < (x_max - 1e-10):
        x = x + precision 
        
        cosh = math.cosh( x/ g_catenary_a)
        y = g_catenary_a * (cosh - 1) 

        catenary_list.append([x, y]) 

    y_tran = g_config["signal_area_size"] / 2
    for p in catenary_list:
        # 上移
        p[1] = y_tran + p[1]

    # 保证从连续性
    circle_list2.reverse()
    #circle_list1.reverse()
    #catenary_list.reverse()

    """
    #调试
    print(1)
    for p in circle_list2:
        print(p[0], end = ',')
        print(p[1])
    print(2)
    for p in catenary_list:
        print(p[0], end = ',')
        print(p[1])
    print(3)
    for p in circle_list1:
        print(p[0], end = ',')
        print(p[1])
    """

    return (circle_list2, catenary_list, circle_list1)

def get_noup_outline(angle):
    """
    旋转获取
    """
    rotate_angle = math.radians(angle)
    cosa = math.cos(rotate_angle)
    sina = math.sin(rotate_angle)

    outline = get_up_outline()
    for shape_list in outline:
        for p in shape_list:
            x = p[0] * cosa - p[1] * sina
            y = p[0] * sina + p[1] * cosa
            p[0] = x
            p[1] = y

    return outline

def get_right_outline():
    return get_noup_outline(270.0)

def get_down_outline():
    return get_noup_outline(180.0)

def get_left_outline():
    return get_noup_outline(90.0)

def get_draw_cmd(): 
    """
    初始环境已经生成
    """

    """
    命令框架
    """
    draw_cmd_head = """add line\n"""
    draw_cmd_tail = """done\n"""
    # 命令格式
    draw_cmd_format = """pick grid %.2f %.2f\n"""

    draw_cmd = ""
    """
    # 上方边框
    up_outline = get_up_outline()
    for shape_list in up_outline:
        for p in shape_list:
            cmd = draw_cmd_format % (p[0], p[1])
            draw_cmd = draw_cmd + cmd
            #print("%.2f,%.2f"  % (p[0], p[1]))

    """
    # 边框 
    outlines = []
    outlines.append(get_right_outline())
    outlines.append(get_down_outline())
    outlines.append(get_left_outline()) 
    outlines.append(get_up_outline())
    
    i = 1
    for outline in outlines: 
        draw_cmd = draw_cmd + ("#%d\n" % i )
        i = i + 1
        for shape_list in outline:
            for p in shape_list:
                cmd = draw_cmd_format % (p[0], p[1])
                draw_cmd = draw_cmd + cmd

    draw_cmd_all = draw_cmd_head + draw_cmd + draw_cmd_tail
    #print(draw_cmd_all)
    return draw_cmd_all

def catenary(): 
    # 定点
    x_c = (g_config["board_size"] / 2) - (g_config["motor_hole_out_r"] * 2)
    y_c = (g_config["board_size"] / 2) - (g_config["signal_area_size"] / 2) - g_config["motor_hole_out_r"]
    
    # a 链表构建
    a_min = 10
    a_max = 100
    step = 1.0 / 100000 
    
    a_nums = int((a_max - a_min)/step)

    # 构造系数列表
    a_list = []
    for i in range(0, a_nums):
        a_list.append(a_min + i*step)

    # 遍历求解系数
    y_last = 0
    for a in a_list:
        cosh = math.cosh( x_c / a)
        y = a * (cosh - 1) 

        if abs(y - y_c) < 1e-5:
            #print("%.4f, %.5f, %.5f" % (x_c, y, a))
            return a


if __name__ == '__main__': 
    # 悬链线系数 需要放在所有参数最后
    #print("#求出悬链线系数, 稍等")
    g_catenary_a = catenary()
    #print("#悬链线系数:%.5f" % g_catenary_a)

    cmd_all = g_scr + get_draw_cmd()
    print(cmd_all)

