#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json
from os import error

# import matplotlib相關套件

import matplotlib.pyplot as plt

StartTime = 3600

def main():
    with open('nccu_output.json') as f:
    # with open('nccu_output_origin.json') as f:
    # with open('nccu_output_noNDN.json') as f:
        print(f.name)
        print('----------------------------------------------------')

        data = json.load(f)

        orderNum = 0

        #紀錄有資料回傳的Order數量
        hasData_return = 0

        totalDelay = 0
        totalDataNum = 0
        total_Fulfill_Num = 0
        total_local_cache = 0

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
                        if arrive_time - startTime < 10:
                            total_Fulfill_Num += 1

                        if arrive_time == startTime:
                            total_local_cache += 1

                        if arrive_time > longest_time:
                            longest_time = arrive_time
                
                if (longest_time - startTime) >= 10:
                    continue
                
                
                if longest_time > startTime:
                    totalDelay = totalDelay + (longest_time-startTime)
                
                hasData_return = hasData_return + 1 

            #     break
            # break
        
        print("totalDataNum = " + str(totalDataNum))
        print("total_Fulfill_Num = " + str(total_Fulfill_Num))
        print("Data hit rate (not include cache) =  " + str((total_Fulfill_Num - total_local_cache)/(totalDataNum - total_local_cache)))
        print("average Delay (per Order) = " + str(totalDelay/hasData_return))
        print("hasData_return = " + str(hasData_return))
        print("orderNum = " + str(orderNum))
        print("total_local_cache = " + str(total_local_cache))
        print('----------------------------------------------------')

    customer_send_connect = 0
    customer_send_disconnect = 0
    customer_on_interest = 0
    
    with open('CustomerApp_log.txt') as f:
        for line in f.readlines():

            words = line.split();
            if float(words[0].strip('s').strip('+')) < StartTime:
                continue

            if line.find('CustomerApp:SendInterest()') != -1 and line.find('Kbucket_connect') != -1 :
                customer_send_connect = customer_send_connect +1
            
            if line.find('CustomerApp:SendInterest()') != -1 and line.find('Kbucket_disconnect') != -1:
                customer_send_disconnect = customer_send_disconnect +1

            if line.find('CustomerApp:OnInterest()') != -1:
                customer_on_interest = customer_on_interest +1

    print("CustomerApp: send Kbucket_connect = " + str(customer_send_connect))
    print("CustomerApp: send Kbucket_disconnect = " + str(customer_send_disconnect))
    print("CustomerApp: recieve Interest = " + str(customer_on_interest))
    print('----------------------------------------------------')


    manage_send_data = 0
    manage_on_interest = 0
    manage_send_interest = 0
    manage_no_match = 0
    manage_ndn_fault = 0

    # NO-match-Data-&-next-Node
    with open('DataManage_log.txt') as f:
        for line in f.readlines():
            words = line.split()

            if float(words[0].strip('s').strip('+')) < StartTime:
                continue

            if line.find('DataManage:OnInterest():') != -1 & line.find('Sending Data packet for') != -1  :
                manage_send_data = manage_send_data +1
            
            if line.find('DataManage:OnInterest():') != -1 & line.find('Received Interest packet for') != -1  :
                manage_on_interest = manage_on_interest +1
            
            if line.find('Query another Node for data') != -1:
                manage_send_interest = manage_send_interest +1

            if line.find('Query another Node for ndnFault_tolerant') != -1:
                manage_ndn_fault = manage_ndn_fault +1
            
            if line.find('NO-match-Data-&-next-Node') != -1:
                manage_no_match = manage_no_match +1
    
    print("DataManage: Sending Data packet for = " + str(manage_send_data))
    print("DataManage: Received Interest packet = " + str(manage_on_interest))
    print("DataManage: Query another Node for data = " + str(manage_send_interest))
    print("DataManage: ndnFault_tolerant = " + str(manage_ndn_fault))
    print("DataManage: NO-match-Data-&-next-Node = " + str(manage_no_match))
    print('----------------------------------------------------')

    origin_send_next_round = 0
    origin_send_data = 0
    origin_send_interest = 0
    origin_no_match = 0
    origin_send_back = 0

    with open('DataManageOrigin_log.txt') as f:
        for line in f.readlines():
            words = line.split()

            if float(words[0].strip('s').strip('+')) < StartTime:
                continue

            if line.find('next round Query:') != -1  :
                origin_send_next_round  = origin_send_next_round +1
            
            if line.find('DataManageOrigin:OnInterest():') != -1 & line.find('Sending Data packet for') != -1  :
                origin_send_data = origin_send_data +1
            
            if line.find('DataManageOrigin:SendInterest():') != -1:
                origin_send_interest = origin_send_interest +1
            
            if line.find('NO-match-Data-&-next-Node') != -1:
                origin_no_match = origin_no_match +1
            
            if line.find('sendBack to SourceNode:') != -1:
                origin_send_back = origin_send_back +1
    
    print("DataManageOrigin: next round Query = " + str(origin_send_next_round))
    print("DataManageOrigin: Sending Data packet for = " + str(origin_send_data))
    print("DataManageOrigin: SendInterest() = " + str(origin_send_interest))
    print("DataManageOrigin: NO-match-Data-&-next-Node = " + str(origin_no_match))
    print("DataManageOrigin: sendBack to SourceNode = " + str(origin_send_back))



# 資料視覺化

    # # 設定圖片大小為長15、寬10
    # plt.figure(figsize=(15,10),dpi=100,linewidth = 2)

    # # NDN錯誤容忍
    # time_arr = []
    # fulfill_data_arr = []

    # time = 0

    # for i in range(0,15):
    #     temp_fulfill_num = 0
        
    #     with open('nccu_output.json') as f:
    #         data = json.load(f)

    #         for node in data:
    #             for orderName in data[node]:
    #                 if "fulfill-order" not in data[node][orderName]:
    #                     continue

    #                 for item in data[node][orderName]["fulfill-order"]:

    #                     test_string = data[node][orderName]["Start-process-Order"]["Time"]
    #                     startTime = float(test_string.strip('+').strip('s'))
    #                     arrive_time = float(data[node][orderName]["fulfill-order"][item])

    #                     if (arrive_time - startTime) >= 10:
    #                         continue

    #                     if arrive_time - startTime <= time:
    #                         temp_fulfill_num += 1
                            
            
    #         time_arr.append(time)
    #         fulfill_data_arr.append(temp_fulfill_num)
    #         temp_fulfill_num = 0

    #         time = time + 0.05
    # # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
    # plt.plot(time_arr,fulfill_data_arr,'s-',color = 'r', label="NDN fault tolerant")

    # # 原版Kademlia
    # time_arr = []
    # fulfill_data_arr = []

    # time = 0

    # for i in range(0,15):
    #     temp_fulfill_num = 0
        
    #     with open('nccu_output_origin.json') as f:
    #         data = json.load(f)

    #         for node in data:
    #             for orderName in data[node]:
    #                 if "fulfill-order" not in data[node][orderName]:
    #                     continue

    #                 for item in data[node][orderName]["fulfill-order"]:

    #                     test_string = data[node][orderName]["Start-process-Order"]["Time"]
    #                     startTime = float(test_string.strip('+').strip('s'))
    #                     arrive_time = float(data[node][orderName]["fulfill-order"][item])

    #                     if (arrive_time - startTime) >= 10:
    #                         continue

    #                     if arrive_time - startTime <= time:
    #                         temp_fulfill_num += 1
            
    #         time_arr.append(time)
    #         fulfill_data_arr.append(temp_fulfill_num)
    #         temp_fulfill_num = 0

    #         time = time + 0.05

    # # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
    # plt.plot(time_arr,fulfill_data_arr,'s-',color = 'g', label="Kademlia")


    # # 純NDN逐跳
    # time_arr = []
    # fulfill_data_arr = []

    # time = 0

    # for i in range(0,15):
    #     temp_fulfill_num = 0
        
    #     with open('nccu_output_noNDN.json') as f:
    #         data = json.load(f)

    #         for node in data:
    #             for orderName in data[node]:
    #                 if "fulfill-order" not in data[node][orderName]:
    #                     continue

    #                 for item in data[node][orderName]["fulfill-order"]:

    #                     test_string = data[node][orderName]["Start-process-Order"]["Time"]
    #                     startTime = float(test_string.strip('+').strip('s'))
    #                     arrive_time = float(data[node][orderName]["fulfill-order"][item])

    #                     if (arrive_time - startTime) >= 10:
    #                         continue

    #                     if arrive_time - startTime <= time:
    #                         temp_fulfill_num += 1
            
    #         time_arr.append(time)
    #         fulfill_data_arr.append(temp_fulfill_num)

    #         time = time + 0.05

    # # 把資料放進來並指定對應的X軸、Y軸的資料，用方形做標記(s-)，並指定線條顏色為紅色，使用label標記線條含意
    # plt.plot(time_arr,fulfill_data_arr,'s-',color = 'b', label="kademlia hop by hop")



    # # 設定圖片標題，以及指定字型設定，x代表與圖案最左側的距離，y代表與圖片的距離

    # plt.title("Python Line chart", x=0.5, y=1.03)

    # # 设置刻度字体大小

    # plt.xticks(fontsize=20)

    # plt.yticks(fontsize=20)

    # # 標示x軸(labelpad代表與圖片的距離)

    # plt.xlabel("time", fontsize=30, labelpad = 15)

    # # 標示y軸(labelpad代表與圖片的距離)

    # plt.ylabel("data hit %", fontsize=30, labelpad = 20)

    # # 顯示出線條標記位置

    # plt.legend(loc = "best", fontsize=20)

    # # 畫出圖片

    # plt.show()


            
                





if __name__ == '__main__':
    main()