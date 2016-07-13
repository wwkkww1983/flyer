#!/usr/bin/env python3
# -*- coding: utf-8 -*-

g_file = "data.dat"

def Parse(file_name):
    print("从%s中读取文件分析." % (file_name))
    
    f = open(file_name, 'rb')
    print("开始分析%s." % file_name)

    print("分析完成.")
    while True:
        pass

    f.close()

if __name__ == "__main__":
    Parse(g_file)


