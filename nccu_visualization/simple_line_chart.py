#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from os import error
# import matplotlib相關套件

import matplotlib.pyplot as plt

from analysis import StartTime

# import字型管理套件




def main():
    
    # 資料視覺化

    # 設定圖片大小為長15、寬10
    plt.figure(figsize=(15,10),dpi=100,linewidth = 2)

    file_arr = ['nccu_output.json', 'nccu_output_origin.json', 'nccu_output_noNDN.json']
    lable_arr = ["NDN fault tolerant", "Kademlia", "kademlia hop by hop"]
    color_arr = ['r', 'g', 'b']

    # K桶大小折線圖數據
    time_arr = [5, 8, 10]
    noNDN = [43.99, 57.86, 65.63]
    NDN = [92.19, 94.10, 95.61]
    kademlia = [65.02, 85.80, 89.27]

    # connect & disconnect 成本數據
    # NDN = [0, 0, 0, 0, 0, 0, 0, 29, 26, 11, 39, 16, 7, 17, 14, 16, 11, 7, 11, 8, 25, 11, 18, 11, 12, 12, 18, 7, 10, 333, 1313, 42, 20, 31, 26, 11, 16, 20, 21, 15, 10, 19, 7, 13, 20, 13, 17, 11, 11, 13, 0, 1, 1, 1, 1, 1, 1, 1, 0]
    # kademlia = [0, 0, 0, 0, 0, 0, 0, 15, 35, 24, 65, 32, 29, 46, 42, 27, 43, 56, 59, 35, 23, 45, 42, 25, 41, 48, 37, 55, 38, 243, 1243, 34, 56, 33, 37, 33, 58, 49, 24, 29, 43, 51, 45, 52, 47, 37, 39, 21, 51, 47, 4, 0, 1, 1, 1, 1, 1, 1, 0]
    # time_arr = []
    # for item in range(0, len(NDN)):
    #     time_arr.append(item*1000)

        

    # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
    

    plt.plot(time_arr,NDN,'o-',color = 'r', label="NDN error control + Hop by Hop Kademlia")

    plt.plot(time_arr,kademlia,'s-',color = 'g', label="kademlia")

    plt.plot(time_arr,noNDN,'^-',color = 'b', label="Hop by hop kademlia")
    
    


    # 設定圖片標題，以及指定字型設定，x代表與圖案最左側的距離，y代表與圖片的距離

    # plt.title("Data Hit Rate", x=0.5, y=1.03)

    # 设置刻度字体大小

    plt.xticks(fontsize=20)

    plt.yticks(fontsize=20)

    # 標示x軸(labelpad代表與圖片的距離)

    plt.xlabel("K-bucket size", fontsize=30, labelpad = 15)

    # 標示y軸(labelpad代表與圖片的距離)

    plt.ylabel("Data hit rate(%)", fontsize=30, labelpad = 20)

    # 顯示出線條標記位置

    plt.legend(loc = "best", fontsize=15)

    # 畫出圖片

    plt.show()




if __name__ == '__main__':
    main()