
#ifndef KADEMLIA_
#define KADEMLIA_

#include <iostream>
#include <set>

class Order
{
private:
    /* data */
    std::string orderName;
    std::string itemType;
    double timeStamp;
    int targetNum;

    //設置order順序流水號 方便進行query function
    int serial_num ;

    std::set<std::string> fulfill_data_list = {};
    Order* nextPtr = NULL;
    std::set<std::string> dataList = {};
    bool terminate = true;
    bool hasSource_node = false;
    std::string sourceNode;
    std::set<std::string> shopList = {};

public:
    Order(std::string order_name, std::string dataString, double timeString, int targetNumber)
    {
    orderName = order_name;
    itemType = dataString;
    timeStamp = timeString;
    targetNum = targetNumber;
    nextPtr = NULL;
    terminate = true;
    serial_num = -1;
    };

    std::string
    getItemType(){return itemType;}

    std::string
    getOrderName(){return orderName;}

    double
    getTimeStamp(){return timeStamp;}

    int
    getTargetNum(){return targetNum;}

    std::set<std::string>
    getDataList(){return dataList;}

    std::set<std::string>
    getShopList(){return shopList;}

    std::set<std::string>
    getFulfill_List(){return fulfill_data_list;}

    std::string
    getSourceNode(){return sourceNode;}

    bool
    getHasSourceNode(){return hasSource_node;}

    int
    getSerial_num(){return serial_num;};

    //用新的datalist整個替換
    void
    replace_Datalist(std::set<std::string> inputList){dataList = inputList;}

    void
    setFulfill_List(std::string input){fulfill_data_list.insert(input);}

    void
    setDataList(std::string input){dataList.insert(input);}

    void
    setShopList(std::string input){shopList.insert(input);}

    void
    setShopList(std::set<std::string> input){shopList = input;}

    void
    setTerminate(bool input){terminate = input;}

    void
    setSourceNode(std::string input){sourceNode = input;}

    void
    setHasSourceNode(bool input){hasSource_node = input;}

    void
    setSerial_num(int input){serial_num = input;}

    bool
    getTerminate(){return terminate;}

    Order* getNext(){return nextPtr;}

    void
    setNext(Order* ptr);

    //將新的order 依照 timeStamp 插入 OrderList中 
    void
    AddOrder(std::string orderName, std::string dataString, double timeString, int targetNumber);

    //將order加入orderList尾端
    Order*
    AddOrder_toTail(std::string orderName, std::string dataString, double timeString, int targetNumber);

    //搜尋整個order資料結構 滿足所需資料的所有order
    bool
    checkDataList(std::string DataName, std::string itemtype);
    
    //確認此筆order是否完成
    bool  
    checkFulFill();

    void
    deleteOrder(Order* preOrder);

    Order* getTail();
    ~Order(){};
};


class Data{

public:
    ~Data()
    {
        head = this;
    }

    void AddData(std::string inputName, std::string inputType);

    Data* GetData(std::string DataName);

    void printAllData();

    Data* GetTail();

    Data *head;
    Data *next = NULL;
    std::string Name = "NULL";
    std::string UserID;
    std::string age;
    std::string item;
    std::string type;
};

class Kademlia{

public:
    Kademlia();

    Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID);

    bool
    GetData(std::string DataName);

    
    std::string
    GetNodeName(){
        return node_name;
    };

    void
    SetData(std::string input, std::string type);

    std::string
    GetNext_Node(std::string TargetNode);

    std::string*
    GetK_bucket(){return k_bucket; };

    int
    XOR(std::string input);

    int
    XOR(std::string input, std::string input2);

    std::string
    GetKId(){
        return KId;
    };

    void
    Set_KBucket(std::string KNode);

    void
    Node_info();

    //以輸入的節點名稱比較其他K桶中資訊，並決定是否將其加入
    std::string
    KBucket_update(std::string sourceNode);

    //將輸入節點從k桶中去除
    void
    KBucket_delete(std::string sourceNode);

    //K桶中有無節點
    bool
    KBucket_hasNode(std::string sourceNode){
        for (int i = 0; i < 15; i++)
        {
            if (k_bucket[i] == sourceNode)
            {
                return true;
            }
            
        }
        return false;
    }

private:
    std::string node_name;
    std::string KId;
    Data *dataList;
    std::string k_bucket[15] = {"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL"};
    int knum = 0;
};

class Guest
{
private:
    Guest* nextPtr = NULL;
    double timestamp = 0;
    std::string recordName = "NULL";
public:
    Guest(std::string recordInput, double timeInput){
        recordName = recordInput;
        timestamp = timeInput;
        nextPtr = NULL;
    };
    ~Guest(){

    };

    Guest*
    getNext(){return nextPtr;};

    double
    getTimeStamp(){return timestamp;};

    std::string
    getRecordName(){return recordName;};

    void
    setNext(Guest* inputPtr){nextPtr = inputPtr;};
};




#endif 