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
#define PS_THRE 1340
#define EXPIRE_TIME 3000000
#define REPORT_FREQ 1.0f

class AP_pool;

struct packet{
        string src;
        string dst;
        timeval time;
        int len;
        packet(string s,string d,timeval t,int l):src(s),dst(d),time(t),len(l){};
};

class client_stats {
        private:
                deque<packet*> mlist;
                std::map<packet*, int> mAMPDU_map;
                void AMPDU_map();
                double mAver;
                double mSTD;
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
                        
};
