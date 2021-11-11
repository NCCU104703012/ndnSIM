#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# node  comment     yPos    xPos
# Node0   NA          10      10 

# srcNode   dstNode     bandwidth   metric  delay   queue
# bandwidth: link bandwidth
# metric: routing metric
# delay:  link delay
# queue:  MaxPackets for transmission queue on the link (both directions)
# Node0       Node1       1Mbps       1       10ms    10


import numpy as np

def main():
    scale = 15

    print("OK")
    arr = arrayA = np.arange(scale*scale)
    # for item in range(0, 49):
    #     arr.append(item)

    arr = np.reshape(arr, (scale, scale))

    print(arr)

    fp = open("nccu_topo225.txt", "a")

    yPos = 0
    xPos = 0

    fp.writelines("router\n")

    for item in range(0, scale*scale):
        output = "Node" + str(item) + "   NA   " + str(yPos+1) + "   " + str(xPos+1) + "\n"
        
        xPos = xPos + 3
        
        
        if (item+1) % scale == 0:
            yPos = yPos + 3
            xPos = 0
        
        fp.writelines(output)

    fp.writelines("link\n")
    
    for item in range(0, scale*scale):

        if item + scale <= scale*scale-1:
            output = "Node" + str(item) + "       Node" + str(item+scale) + "       100Mbps       " + "1       " + "10ms    " + "100" + "\n"
            fp.writelines(output)

        if (item+1) % scale != 0:
            output = "Node" + str(item) + "       Node" + str(item+1) + "       100Mbps       " + "1       " + "10ms    " + "100" + "\n"
            fp.writelines(output)
            

        
        


    

if __name__ == '__main__':

    main()
    