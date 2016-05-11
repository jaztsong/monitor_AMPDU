#include <iostream>
#include<vector>
#include<deque>
#include<cmath>
#include<math.h>
#include "./wifipcap/wifipcap.h"
//Debug
/* #define DEBUG */

#ifdef DEBUG
#define Debug( x   ) std::cout << x <<endl;
#else
#define Debug( x   ) 
#endif
using namespace std;

#define GAP_MPDU 80
#define CLIENT_WIN 3000000
#define AMPDU_WIN 0.5f
#define PS_THRE 1300
#define EXPIRE_TIME 3000000
#define REPORT_FREQ 1.0f

class AP_pool;

struct packet{
        string src;
        string dst;
        timeval time;
        int len;
        bool ampdu_has;
        bool ampdu_last;
        packet(string s,string d,timeval t,int l,bool has, bool last):src(s),dst(d),time(t),len(l),ampdu_has(has),ampdu_last(last){};
};

class client_stats {
        private:
                deque<packet*> mlist;
                std::map<packet*, int> mAMPDU_map;
                void AMPDU_map();
                double mAver;
                double mSTD;
                int mMax;
                int mNum;
                string mMac_address;
                timeval mClientUpTime;
                AP_pool* mPool;
                int time_index(timeval t);

        public:
                client_stats(string mac_addr);
                ~client_stats();
                void setPool(AP_pool* p);
                void add_packet(struct packet* p);
                void getAMPDU_stats();
                void updateList();
                double getAver();
                double getSTD();
                int getMax();
                int getNum();
                timeval getUpTime();

};
class AP_stats {
        private:
                map<string, client_stats*> mClients;
                string mMac_address;
                timeval mAPUpTime;
                string mSSID;
                AP_pool* mPool;
                
        public:
                AP_stats(string mac);
                void setSSID(string ssid);
                void setPool(AP_pool* p);
                AP_pool* getPool();
                void add_packet(packet* p);
                void updateClients();
                timeval getUpTime();
                void dump_report();
                map<string, client_stats*>* getClients();
                        
};

class AP_pool {
        private:
                map<string, AP_stats*> mAPs;
                string mName;
                timeval mUpTime;
                
                
        public:
                AP_pool(string name);
                AP_stats* find_AP(string mac);
                void UpdateSSID(string mac,string ssid);
                timeval getUpTime();
                void add_packet(packet* p);
                void updateAPs();
                void report();
                void dump_stat();
                        
};
