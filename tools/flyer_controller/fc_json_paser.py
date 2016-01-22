#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json

class FCJsonPaser():
    def __init__(self, jsonName):
        f = open(jsonName, "r")
        dataStr = f.read()

        """
        for l in dataStr:
            print(l, end='')
        """

        self.mJson = json.loads(dataStr)

        """
        print(self.mJson.keys())
        print(self.mJson["Axis"]["Scale"])
        print(self.mJson["Axis"]["ArrowScale"])
        """

    def GetValue(self, keyList):
        v = self.mJson[keyList[0]]
        #print(v)

        maxIndex = len(keyList)
        for i in range(1, maxIndex):
            #print(keyList[i])
            v = v[keyList[i]]

        return v

    def ToDict(self):
        return self.mJson

if __name__ == '__main__': 
    gJsonName = 'axis.json'
    jsonPaser = FCJsonPaser(gJsonName)
    axisScale = jsonPaser.GetValue(("Axis", "Scale"))
    arrowScale = jsonPaser.GetValue(("Axis", "ArrowScale"))
    print('测试通过')

