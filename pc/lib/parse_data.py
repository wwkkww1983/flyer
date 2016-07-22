#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import struct
import binascii

g_file = "data.dat"
g_frame_length = 32

class FParser():
    def __init__(self, file_name):
        self.mFile = open(file_name, 'rb')

    def __del__(self):
        self.mFile.close()

    def ParseAFrame(self): 
        frame = self.mFile.read(g_frame_length) 

        print("帧内容:")
        # 字节打印
        i = 0
        for b in frame:
            print("%02x" % b, end = '')
            i = i + 1
            if 4 == i:
                i = 0
                print()

        print("帧中crc32:")
        # 解包
        parsed_frame = struct.unpack('>HHHIHHHHHHHHHI', frame) 
        f_crc32 = parsed_frame[13]
        print("%0x" % f_crc32)
        
        #computed_crc32 = (binascii.crc32(frame) & 0xffffffff)
        computed_crc32 = (binascii.crc32(b"\xff\xff\xff\xff" + frame) & 0xffffffff)
        #computed_crc32 = binascii.crc32(b"\xff\xff\xff\xff" + frame)
        print("计算的crc32:")
        print("%0x" % computed_crc32)

def Parse(file_name):
    print("从%s中读取文件分析." % (file_name))
    
    f = open(file_name, 'rb')
    print("开始分析%s." % file_name)
    print()

    frame = f.read(g_frame_length)
    for b in frame:
        print("%02x," % b, end = '')

    parsed_frame = struct.unpack('>HHHIHHHHHHHHHI', frame)
    f_length = parsed_frame[0]
    f_fill = parsed_frame[1]
    f_type = parsed_frame[2]
    f_time = parsed_frame[3]
    f_accel_x = parsed_frame[4]
    f_accel_y = parsed_frame[5]
    f_accel_z = parsed_frame[6]
    f_gyro_x = parsed_frame[7]
    f_gyro_y = parsed_frame[8]
    f_gyro_z = parsed_frame[9]
    f_compass_x = parsed_frame[10]
    f_compass_y = parsed_frame[11]
    f_compass_z = parsed_frame[12]
    f_crc32 = parsed_frame[13]
    print()
    print(parsed_frame)

    print(f_length)
    print(f_fill)
    print(f_type)
    print(f_time)
    print(f_accel_x)
    print(f_accel_y)
    print(f_accel_z)
    print(f_gyro_x)
    print(f_gyro_y)
    print(f_gyro_z)
    print(f_compass_x)
    print(f_compass_y)
    print(f_compass_z)
    print(f_crc32)

    #parsed_frame = struct.unpack('HHHIHHHHHHHHHI', frame)
    #print(parsed_frame)

    print()
    f.close()

if __name__ == "__main__":
    parser = FParser(g_file)
    parser.ParseAFrame()
    c = input("分析完成,输入回车键退出")

