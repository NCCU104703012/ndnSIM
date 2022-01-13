#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from os import error
# import matplotlib相關套件

import matplotlib.pyplot as plt

from analysis import StartTime

def main():

     # 設定圖片大小為長15、寬10
    plt.figure(figsize=(15,10),dpi=100,linewidth = 2)

    file_arr = ['rate-trace_5kbuk_half.txt', 'rate-trace_origin_10kbuk_half.txt', 'nccu_output_noNDN.json']
    lable_arr = ["NDN error control + Hop by Hop Kademlia(K=5)", "Kademlia(K=10)", "kademlia hop by hop(K=10)"]
    label_marker_arr = ["o-", "s-", "^-"]
    color_arr = ['r', 'g', 'b']



    for index in range(0,2):

        Ininterest = 0
        Indata = 0

        time_arr = []
        PacketRaw_arr = []

        interest_dict = dict()

        with open(file_arr[index]) as f:
        # with open('rate-trace_origin_10kbuk_half.txt') as f:

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
                    if int(Time) >= 8000 and int(Time) <= 50000:
                        if Time not in interest_dict and Type == 'InInterests':
                            interest_dict[Time] = int(PacketRaw)
                        elif Type == 'InInterests':
                            interest_dict[Time] = interest_dict[Time] + int(PacketRaw)

                        if Type == 'InInterests':
                            Ininterest = Ininterest + int(PacketRaw)
                        
                        if Type == 'InData':
                            Indata = Indata + int(PacketRaw)
                
            print("in interest : " + str(Ininterest))
            print("in Data : " + str(Indata))
        
        total = 0

        for item in interest_dict.items():
            time_arr.append(int(item[0]))
            # total = total + int(item[1])
            # PacketRaw_arr.append(total)
            PacketRaw_arr.append(int(item[1]))

        print(time_arr)
        print(PacketRaw_arr)
            

        # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
        plt.plot(time_arr,PacketRaw_arr,label_marker_arr[index],color = color_arr[index], label=lable_arr[index])

    # 設定圖片標題，以及指定字型設定，x代表與圖案最左側的距離，y代表與圖片的距離

    plt.title("Rate trace Line chart", x=0.5, y=1.03)

    # 设置刻度字体大小

    plt.xticks(fontsize=20)

    plt.yticks(fontsize=20)

    # 標示x軸(labelpad代表與圖片的距離)

    plt.xlabel("Time(s)", fontsize=30, labelpad = 15)

    # 標示y軸(labelpad代表與圖片的距離)

    plt.ylabel("Physical link interest number", fontsize=30, labelpad = 20)

    # 顯示出線條標記位置

    plt.legend(loc = "best", fontsize=15)

    # 畫出圖片

    plt.show()
                    
                
                 




if __name__ == '__main__':
    main()