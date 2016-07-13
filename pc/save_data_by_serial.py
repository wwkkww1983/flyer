#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import time
import serial
#import serial.tools.list_ports

g_file = "data.dat"
g_com = "COM4"
g_baudrate = 115200
g_frame_size = 1

# 200包/s
max_counts = 200 * 10

def Capture(file_name, ser):
    print("从%s:%d捕获." % (ser.portstr, ser.baudrate))
    print("存入%s." % (file_name)) 
    
    ser.open()
    f = open(file_name, 'wb')
    count = 0

    print("开始记录.")
    while count < max_counts:
        line_bytes = ser.read(g_frame_size)
        print(line_bytes, end = '')
        print("\t%5.2f%%" % (100 * count / max_counts))
        f.write(line_bytes)
        #f.flush()
        count = count + 1

    ser.close()
    f.close()

    print("记录完成.")
    while True:
        pass

if __name__ == "__main__":
    ser = serial.Serial()
    ser.port = g_com
    ser.baudrate = g_baudrate
    Capture(g_file, ser)

