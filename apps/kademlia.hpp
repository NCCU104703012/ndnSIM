
#ifndef KADEMLIA_
#define KADEMLIA_

#include <iostream>
#include <set>
#include <bitset>
#include <cstring>

#include <stdio.h>
#include <sqlite3.h>



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
    checkDataList(std::string DataName);
    
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

    void AddData(std::string inputName, std::string k_ID);

    void AddData(std::string inputName, std::string inputType, int Empty);


    Data* GetData(std::string DataName);

    Data* GetData(std::string DataName, std::string k_ID);

    void printAllData();

    Data* GetTail();

    int
    XOR(std::string input, std::string input2);

    void
    update_nextHop(std::string inputNode);

    void
    SetClosest_Node(){
        std::size_t hashData = std::hash<std::string>{}(this->Name);
        std::string binaryData = std::bitset<8>(hashData).to_string();

        for (int i = 0; i < 3; i++)
        {
            if (nextHop_list[i] != "NULL")
            {
                if (XOR(nextHop_list[i], binaryData) < XOR(closest_node, binaryData))
                {
                    closest_node = nextHop_list[i];
                }
            }
        }
        
    };

    

    Data *head;
    Data *next = NULL;
    std::string Name = "NULL";
    std::string UserID;
    std::string age;
    std::string item;
    std::string type;

    std::string closest_node = "NULL";
    double lifeTime = 0;

    int reply_count = 0;
    int target_reply_count = 0;
    std::string nextHop_list[3] = {"NULL", "NULL", "NULL"};
    std::string timeout_check[3] = {"NULL", "NULL", "NULL"};


    //資料庫的輸出儲存變數
    sqlite3* db;

    //將資料加入資料庫中
    static int DB_addDATA(void *NotUsed, int argc, char **argv, char **azColName){
        // int i;
        // for(i=0; i<argc; i++){
        //     //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        //     std::cout << argv[i] << "  ";
        // }
        // printf("\n");
        std::cout << "success add data : " << argv[0] << " " << argv[1] << "\n";
        return 0;
    }

    static int DB_getDATA(void *NotUsed, int argc, char **argv, char **azColName){
        int *flag = (int*)NotUsed;
        *flag = 1;
        std::cout << "success get data : " << argv[0] << " " << argv[1] << "\n";
        return 0;
    }

    static int DB_DeleteData(void *NotUsed, int argc, char **argv, char **azColName){
        std::cout << "success delete data : " << argv[0] << " " << argv[1] << "\n";
        return 0;
    }

    static int DB_NULL(void *NotUsed, int argc, char **argv, char **azColName){
        return 0;
    }

};

class Kademlia{

public:
    Kademlia();

    Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID, sqlite3* inputDB);

    bool
    GetData(std::string DataName);

    
    std::string
    GetNodeName(){
        return node_name;
    };

    void
    SetData(std::string input, std::string type);

    std::string
    GetNext_Node(std::string TargetNode, int output_num, std::string SourceNode);

    std::string*
    GetK_bucket(int distance){
        if (distance > 8 || distance < 0)
        {
            std::cout << "error: GetK_bucket() distance = " << distance << "\n";
        }

        if (distance >= 6)
        {
            return k_bucket;
        }
        else if (distance >= 4)
        {
            return k_bucket4;
        }
        return k_bucket8; 
    };

    Data*
    GetDataList(){return dataList;};

    int
    GetSameBits(std::string intputKID);

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
    std::pair<std::string, std::string>
    KBucket_update(std::string sourceNode, int distance);

    //將輸入節點從k桶中去除
    void
    KBucket_delete(std::string sourceNode);

    //K桶中有無節點
    bool
    KBucket_hasNode(std::string sourceNode){
        for (int i = 0; i < GetK_bucket_size(); i++)
        {
            if (k_bucket[i] == sourceNode)
            {
                return true;
            }
            if (k_bucket4[i] == sourceNode)
            {
                return true;
            }
            if (k_bucket8[i] == sourceNode)
            {
                return true;
            }
        }
        return false;
    }

    //從datalist中刪除資料
    void
    Delete_data(std::string DataName);

    //從datalist中刪除資料
    void
    Delete_data_query(std::string DataName);

    //針對輸入節點，比較所有更接近的資料，回傳整理的字串
    std::string
    Transform_Data(std::string thisNode, std::string targetNode);

    //print K桶內容
    void
    print_Kbucket(){
        std::cout << "k_bucket: ";
        for (int i = 0; i < GetK_bucket_size(); i++)
        {
            if (k_bucket[i] != "NULL")
            {
                std::cout << k_bucket[i] + "|";
            }
        }
         std::cout << "\n";
        std::cout << "k_bucket4: ";
         for (int i = 0; i < GetK_bucket_size(); i++)
        {
            if (k_bucket4[i] != "NULL")
            {
                std::cout << k_bucket4[i] + "|";
            }
        }
        std::cout << "\n";
        std::cout << "k_bucket8: ";
         for (int i = 0; i < GetK_bucket_size(); i++)
        {
            if (k_bucket8[i] != "NULL")
            {
                std::cout << k_bucket8[i] + "|";
            }
        }
         std::cout << "\n";
    };

    //return queryList中相同名稱的Data*
    Data*
    GetQueryItem(std::string input){
        Data* outputPtr = queryList->next;
        while (outputPtr != NULL)
        {
            if (outputPtr->Name == input)
            {
                return outputPtr;
            }
            outputPtr = outputPtr->next;
        }

        if (outputPtr == NULL)
        {
            return NULL;
        }
        else
        {
            return outputPtr;
        }
    };

    //輸入節點訊息，代替querylist中節點
    void
    update_nextHop(std::string inputNode);

    int
    GetK_bucket_size();

    sqlite3*
    getDBptr(){return db;};

    bool
    GetisOnline(){return isOnline;};

    void
    SetisOnline(bool input){isOnline = input;};

    void
    Init_Kbucket();

    //將目前K桶狀態存入資料庫中
    void
    SetK_bucket_to_DB();


    //用在原版kad使用，紀律這筆資料未來應該query的節點list
    Data *queryList;

private:
    std::string node_name;
    std::string KId;
    Data *dataList;
    std::string k_bucket[15] = {"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL"};
    std::string k_bucket4[15] = {"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL"};
    std::string k_bucket8[15] = {"NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL", "NULL"};

    //資料庫的輸出儲存變數
    sqlite3* db;

    //節點是否上下線
    bool isOnline = true;

    
    //返回符合節點字串，用 | 隔開
    static int DB_getDATA_string(void *NotUsed, int argc, char **argv, char **azColName){
        char **result_str = (char **)NotUsed;
        std::string s = "|";
        strcat(*result_str,argv[1]);
        strcat(*result_str, &s[0]);

        // std::cout << "success get data : " << argv[0] << " " << argv[1] << "\n";
        return 0;
    }

};

class Guest
{
private:
    Guest* nextPtr = NULL;
    double timestamp = 0;
    std::string recordName = "NULL";
    int serial_num = 0;
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

    int
    getSerialNum(){return serial_num;};

    std::string
    getRecordName(){return recordName;};

    void
    setNext(Guest* inputPtr){nextPtr = inputPtr;};

    void
    setSerialNum(int input){serial_num = input;};
};




#endif 