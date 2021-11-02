#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import json

def main():
    with open('nccu_output_origin.json') as f:
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
                        fulfill_count = fulfill_count + 1
                        arrive_time = float(data[node][orderName]["fulfill-order"][item].strip('+').strip('s'))
                        if arrive_time > longest_time:
                            longest_time = arrive_time
                
                if (longest_time - startTime) >= 10:
                    continue
                
                total_Fulfill_Num = total_Fulfill_Num + fulfill_count
                if longest_time > startTime:
                    totalDelay = totalDelay + (longest_time-startTime)
                
                hasData_return = hasData_return + 1 

            #     break
            # break
        
        print("totalDataNum = " + str(totalDataNum))
        print("total_Fulfill_Num = " + str(total_Fulfill_Num))
        print("average Delay = " + str(totalDelay/hasData_return))
        print("hasData_return = " + str(hasData_return))
        print("orderNum = " + str(orderNum))



if __name__ == '__main__':
    main()