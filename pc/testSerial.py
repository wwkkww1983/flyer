#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import serial
import serial.tools.list_ports

gData = b"hello\x7e\x16"

def test(com, baudrate):
    ser = serial.Serial()
    ser.port = com
    ser.baudrate = baudrate
    ser.open()
    print("%s:%d" % (ser.portstr, ser.baudrate))

    while True:
        line_bytes = ser.readline()
        """
        # 是期望的字符串
        if wanted_str == line:
            func()
        """
        line_str = line_bytes.decode('ascii')
        #line_str = line_bytes.decode('utf-8')
        print(line_str, end = '')

    ser.close()

if __name__ == "__main__":
    print('all serial:')
    allSerial = serial.tools.list_ports.comports()
    for s in allSerial:
        print(s[0])
    print()

    test("COM4", 115200)

