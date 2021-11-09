#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# +5.000000000s 0 CustomerApp:SendQuery(): [DEBUG] Start-process-Order newRecord_Node0_9

# +5.053679998s 0 CustomerApp:OnData(): [DEBUG] Order-Complete newRecord_Node0_9

# +5.053679998s 0 CustomerApp:OnData(): [DEBUG] fulfill-order newRecord_Node0_9 data-name 00000001

# +16.140871992s 15 DataStore:OnInterest(): [DEBUG] Store-data-complete newRecord_Node0_7

import json

def main():
    f = open("nccu1107.txt")
    data = dict()
    for line in f.readlines():
        
        words = line.split();
        if len(words) < 5:
            continue
        if len(words[0]) < 13 :
            continue
        if len(words[0]) > 17:
            continue

        if words[4] == 'Start-process-Order':
            if words[1] not in data:
                data[words[1]] = dict()
            if words[5] not in data[words[1]]:
                data[words[1]][words[5]] = dict()

            data[words[1]][words[5]][words[4]] = dict()
            data[words[1]][words[5]][words[4]]["Time"] = words[0]
        
        if words[4] == 'Order-Complete':
            if words[1] not in data:
                data[words[1]] = dict()
            if words[5] not in data[words[1]]:
                data[words[1]][words[5]] = dict()

            data[words[1]][words[5]][words[4]] = dict()
            data[words[1]][words[5]][words[4]]["Time"] = words[0]
        
        if words[4] == 'fulfill-order':
            if words[4] not in data[words[1]][words[5]]:
                data[words[1]][words[5]][words[4]] = dict()
            data[words[1]][words[5]][words[4]][words[7]] = words[0]

        if words[4] == 'MicroService_timeout':
            if words[4] not in data[words[1]][words[5]]:
                data[words[1]][words[5]][words[4]] = dict()
            data[words[1]][words[5]][words[4]]["Time"] = words[0]

        if words[4] == 'setDataList':
            if words[4] not in data[words[1]][words[7]]:
                data[words[1]][words[7]][words[4]] = list()
            data[words[1]][words[7]][words[4]].append(words[5])
                
            
                
            

        # +1.600000000s 3 CustomerApp:SendQuery(): [DEBUG] setDataList 00000001 in-order newRecord_Node3_0
        # 

        # if words[4] == 'Store-data-complete':
        #     if words[1] not in data:
        #         data[words[1]] = dict()
        #     if words[5] not in data[words[1]]:
        #         data[words[1]][words[5]] = dict()
            
        #     data[words[1]][words[5]][words[4]] = words[0]
            
            
            
            
    print(json.dumps(data,sort_keys=True, indent=4))
    f.close()

    file = open('nccu_output_origin.json', "w")
    json.dump(data, file)
    file.close()

if __name__ == '__main__':
    main()