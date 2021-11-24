#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from os import error
# import matplotlib相關套件

import matplotlib.pyplot as plt

from analysis import StartTime

def main():
    Ininterest = 0
    Indata = 0

    with open('rate-trace.txt') as f:
        for line in f.readlines():
            words = line.split();
            if words[0] == 'Time':
                continue
            try:
                Time = words[0]
                Node = words[1]
                FaceId	= words[2]
                FaceDescr = words[3]
                Type = words[4]
                Packets = words[5]
                Kilobytes = words[6]
                PacketRaw = words[7]
                KilobytesRaw = words[8]
            except :
                continue

            Time = words[0]
            Node = words[1]
            FaceId	= words[2]
            FaceDescr = words[3]
            Type = words[4]
            Packets = words[5]
            Kilobytes = words[6]
            PacketRaw = words[7]
            KilobytesRaw = words[8]

            
            
            if FaceDescr.find('netdev://') != -1:
                if int(Time) > 5000 and int(Time) < 12000:
                    if Type == 'InInterests':
                        Ininterest = Ininterest + int(PacketRaw)
                    
                    if Type == 'InData':
                        Indata = Indata + int(PacketRaw)
            
        print("in interest : " + str(Ininterest))
        print("in Data : " + str(Indata))
                        
                        
                    
                
                 




if __name__ == '__main__':
    main()