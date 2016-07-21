#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import serial
#import serial.tools.list_ports

g_file = "flyer.dat"
g_com = "COM1"
g_baudrate = 115200
g_frame_size = 32
g_file_begin_line = 11

# 200包/s
max_counts = 200 * 60 * 30

def Capture(file_name, ser):
    print("从%s:%d捕获." % (ser.portstr, ser.baudrate))
    print("存入%s." % (file_name)) 
    
    ser.open()
    f = open(file_name, 'wb')

    print("连接四轴串口,并给它上电.")
    i = 1
    while True:
        line = ser.readline()
        if g_file_begin_line == i:
            break
        line_str = line.decode('utf8')
        print("%d," % i, end = '')
        print(line_str, end = '')
        i = i + 1

    count = 0 
    print("开始记录.")
    while count < max_counts:
        line_bytes = ser.read(g_frame_size)
        i = i + 1

        # 打印频率为s
        if 0 == count % 200:
            for b in line_bytes:
                print("%02x," % b, end = '')
            print("\t%5.2f%%" % (100 * count / max_counts))

        f.write(line_bytes)
        #f.flush()
        count = count + 1

    ser.close()
    f.close()

    c = input("记录完成,输入任意键退出")

if __name__ == "__main__":
    ser = serial.Serial()
    ser.port = g_com
    ser.baudrate = g_baudrate
    Capture(g_file, ser)

