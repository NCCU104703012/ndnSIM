#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from os import error
# import matplotlib相關套件

import matplotlib.pyplot as plt

from analysis import StartTime

# import字型管理套件


dict
def json_process(file):
    output = dict()

    with open(file) as f:
        print(f.name)
        print('----------------------------------------------------')

        data = json.load(f)

        orderNum = 0

        #紀錄有資料回傳的Order數量
        hasData_return = 0

        totalDelay = 0
        totalDataNum = 0
        total_Fulfill_Num = 0

        for node in data:
            # if node != "44":
            #     continue
            
            for orderName in data[node]:

                #沒有Query任何資料，不計算數量
                if "setDataList" not in data[node][orderName]:
                    continue

                orderNum = orderNum + 1
                # print(data[node][orderName])

                dataList_count = 0
                fulfill_count = 0
                longest_time = 0

                # print(data[node][orderName]["Start-process-Order"]["Time"])
                test_string = data[node][orderName]["Start-process-Order"]["Time"]
                startTime = float(test_string.strip('+').strip('s'))

                #計算有多少資料被加入dataList，Micro Order先不算
                dataList = data[node][orderName]["setDataList"]
                for item in dataList:
                    if len(item) > 8 :
                        dataList_count = dataList_count + 1
                
                totalDataNum = totalDataNum + dataList_count
                
                if "fulfill-order" not in data[node][orderName]:
                    print(node + " " + orderName)
                    continue
                
                

                for item in data[node][orderName]["fulfill-order"]:
                    if len(item) > 8 :
                        arrive_time = float(data[node][orderName]["fulfill-order"][item])
                        if arrive_time - startTime < 15:
                            total_Fulfill_Num += 1

                        if arrive_time > longest_time:
                            longest_time = arrive_time
                
                if (longest_time - startTime) >= 15:
                    continue
                
                if longest_time > startTime:
                    totalDelay = totalDelay + (longest_time-startTime)
                
                hasData_return = hasData_return + 1 
        
        print("totalDataNum = " + str(totalDataNum))
        print("total_Fulfill_Num = " + str(total_Fulfill_Num))
        print("Data hit rate =  " + str(total_Fulfill_Num/totalDataNum))
        print("average Delay = " + str(totalDelay/hasData_return))
        print("hasData_return = " + str(hasData_return))
        print("orderNum = " + str(orderNum))
        print('----------------------------------------------------')

        output["totalDataNum"] = totalDataNum
        output["total_Fulfill_Num"] = total_Fulfill_Num
        output["totalDelay"] = totalDelay
        output["hasData_return"] = hasData_return
        output["orderNum"] = orderNum

        return output

def main():
    
    # 資料視覺化

    # 設定圖片大小為長15、寬10
    plt.figure(figsize=(15,10),dpi=100,linewidth = 2)

    file_arr = ['nccu_output.json', 'nccu_output_origin.json', 'nccu_output_noNDN.json']
    lable_arr = ["NDN fault tolerant", "Kademlia", "kademlia hop by hop"]
    color_arr = ['r', 'g', 'b']

    # 累計資料份數，以及時間 折線圖
    # for index in range(0,3):
        
    #     time_arr = []
    #     fulfill_data_arr = []

    #     time = 5000
    #     time_gap = 100

    #     for i in range(0,400):
    #         temp_fulfill_num = 0
            
    #         with open(file_arr[index]) as f:
    #             data = json.load(f)

    #             for node in data:
    #                 for orderName in data[node]:
    #                     if "fulfill-order" not in data[node][orderName]:
    #                         continue

    #                     for item in data[node][orderName]["fulfill-order"]:

    #                         test_string = data[node][orderName]["Start-process-Order"]["Time"]
    #                         startTime = float(test_string.strip('+').strip('s'))
    #                         arrive_time = float(data[node][orderName]["fulfill-order"][item])

    #                         if (arrive_time - startTime) >= 15:
    #                             continue

    #                         # if arrive_time <= time and arrive_time > time - 100:
    #                         #     temp_fulfill_num += 1
                            
    #                         if arrive_time <= time :
    #                             temp_fulfill_num += 1
                                
                
    #             time_arr.append(time)
    #             fulfill_data_arr.append(temp_fulfill_num)
                

    #             time = time + time_gap

    #     # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
    #     plt.plot(time_arr,fulfill_data_arr,'s-',color = color_arr[index], label=lable_arr[index])
    
    # 累計fulfill資料 以及其delay 折線圖
    # for index in range(0,3):
    #     json_info = json_process(file_arr[index])
        
    #     time_arr = []
    #     fulfill_data_arr = []

    #     time = 0
    #     time_gap = 0.05

    #     for i in range(0,40):
    #         temp_fulfill_num = 0
            
    #         with open(file_arr[index]) as f:
    #             data = json.load(f)



    #             for node in data:
    #                 for orderName in data[node]:
    #                     if "fulfill-order" not in data[node][orderName]:
    #                         continue

    #                     for item in data[node][orderName]["fulfill-order"]:

    #                         test_string = data[node][orderName]["Start-process-Order"]["Time"]
    #                         startTime = float(test_string.strip('+').strip('s'))
    #                         arrive_time = float(data[node][orderName]["fulfill-order"][item])

    #                         if (arrive_time - startTime) >= 15:
    #                             continue
                            
    #                         if arrive_time - startTime <= time and arrive_time - startTime > time - time_gap :
    #                             temp_fulfill_num += 1

    #                         # if arrive_time - startTime <= time :
    #                         #     temp_fulfill_num += 1
                                
                
    #             time_arr.append(time)

    #             fulfill_data_arr.append(temp_fulfill_num/json_info["totalDataNum"] * 100)
    #             # fulfill_data_arr.append(temp_fulfill_num)
                
    #             time = time + time_gap
                
    #     # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
    #     plt.plot(time_arr,fulfill_data_arr,'s-',color = color_arr[index], label=lable_arr[index])


    # 區間或累計資料命中率
    for index in range(0,3):
        
        time_arr = []
        fulfill_data_arr = []

        time = 5000
        time_gap = 1000

        for i in range(0,40):
            temp_fulfill_num = 0
            temp_setData_num = 0
            
            with open(file_arr[index]) as f:
                data = json.load(f)

                for node in data:
                    
                    for orderName in data[node]:
                        #沒有Query任何資料，不計算數量
                        if "setDataList" not in data[node][orderName]:
                            continue
                        
                        #計算有多少資料被加入dataList，Micro Order先不算
                        dataList = data[node][orderName]["setDataList"]
                        test_string = data[node][orderName]["Start-process-Order"]["Time"]
                        startTime = float(test_string.strip('+').strip('s'))

                        for item in dataList:
                            if len(item) > 8 and startTime <= time and startTime > time - time_gap:
                            # if len(item) > 8 and startTime <= time:
                                temp_setData_num = temp_setData_num + 1
                        
                                if "fulfill-order" not in data[node][orderName]:
                                    continue

                                if item in data[node][orderName]["fulfill-order"] :
                                    temp_fulfill_num += 1

                if temp_setData_num == 0:
                    temp_setData_num = 1
                
                time_arr.append(time)
                fulfill_data_arr.append(temp_fulfill_num/temp_setData_num * 100)
                # fulfill_data_arr.append(temp_setData_num)
                # fulfill_data_arr.append(temp_fulfill_num)
                

                time = time + time_gap

        # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
        plt.plot(time_arr,fulfill_data_arr,'s-',color = color_arr[index], label=lable_arr[index])
    
    


    # 設定圖片標題，以及指定字型設定，x代表與圖案最左側的距離，y代表與圖片的距離

    plt.title("Python Line chart", x=0.5, y=1.03)

    # 设置刻度字体大小

    plt.xticks(fontsize=20)

    plt.yticks(fontsize=20)

    # 標示x軸(labelpad代表與圖片的距離)

    plt.xlabel("time", fontsize=30, labelpad = 15)

    # 標示y軸(labelpad代表與圖片的距離)

    plt.ylabel("data hit rate", fontsize=30, labelpad = 20)

    # 顯示出線條標記位置

    plt.legend(loc = "best", fontsize=20)

    # 畫出圖片

    plt.show()




if __name__ == '__main__':
    main()