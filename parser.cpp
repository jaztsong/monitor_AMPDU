#include "parser.h"

using namespace std;

template <typename T>
inline double average(vector<T> v) {  
        T sum=0;
        for(uint16_t i=0;i<v.size();i++)
                sum+=v[i];
        return double(sum/float( v.size() ));
}
//DEVIATION
double deviation(vector<double> v, double ave) {
        double E=0;
        for(uint16_t i=0;i<v.size();i++)
                E+=(v[i] - ave)*(v[i] - ave);
        return sqrt(1/float( v.size() )*E);
}

///////////////////////////////////////////////////////////////////////////
/////////////////////client_stats/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
client_stats::client_stats(string mac_addr):mClientUpTime(TIME_NONE){
        mlist.clear();
        mAMPDU_map.clear();
        mMac_address = mac_addr;
}

void client_stats::add_packet(struct packet* p){
        /* Debug("client_stats "<<p->dst<<" ::Add Packet "<<p->time<<" "<<p->len); */
        mlist.push_back(p);
        while (mlist.size()>0&& ( p->time - mlist.front()->time ) > CLIENT_WIN ){
                /* Debug("client_stats "<<"pop out "<<mlist.size()<<"\t"<<mlist.front()->time<<"\t"<<( p->time )<<"\t"<<( ( p->time - mlist.front()->time ) )); */
                mlist.pop_front();
        }
        mClientUpTime = p->time;
}

void client_stats::updateList(){
        while (mlist.size()>0&& ( mPool->getUpTime() - mlist.front()->time ) > CLIENT_WIN ){
                /* Debug("client_stats updateList "<<"pop out "<<mlist.size()<<"\t"<<mlist.front()->time<<"\t"<<mPool->getUpTime() <<"\t"<<( ( mPool->getUpTime()  - mlist.front()->time ) )); */
                mlist.pop_front();
        }
}
client_stats::~client_stats(){
        deque<packet*>().swap(mlist);
        map<packet*, int>().swap(mAMPDU_map);
}

void client_stats::AMPDU_map() {
        updateList();
        /* Debug("client_stats: AMPDU_map") */
        if(mlist.empty()){
                Debug("***EMPTY client list***");
                return ;
        }
        mAMPDU_map.clear();
        int num=1;
        timeval pre_t= mlist[0]->time;
        for(deque<packet*>::iterator it=mlist.begin();it!=mlist.end();++it){
                Debug("AMPDU_map: packet "<<( *it )->time<<" "<<( *it )->len<<" size: "<<mlist.size());
                if(( *it )->len > PS_THRE){
                        Debug("AMPDU_map: time cmp "<<( *it )->time<<" - "<<pre_t<<" = "<<( ( *it )->time - pre_t ));
                        if(( ( *it )->time - pre_t )<GAP_MPDU && ( *it )->time - pre_t>0){
                                num++;
                        }
                        else{
                                Debug("AMPDU_map: add bundle "<<( *it )->time<<"  "<<num);
                                mAMPDU_map.insert(pair<packet*,int>(*it,num));
                                Debug("AMPDU_map: size "<<mAMPDU_map.size());
                                num=1;
                        }
                }
                pre_t = ( *it )->time;
        }
}
int client_stats::time_index(timeval t){
       int new_sec =  t.tv_sec - 10*int( t.tv_sec/10 );
       int result=int( ( new_sec + float(t.tv_usec/1000000.0f) )/AMPDU_WIN );
        /* printf(" time_index %d.%06d floor %.6f  = %f result %d\n",t.tv_sec,t.tv_usec,new_sec+ float(t.tv_usec/1000000.0f),( new_sec + float(t.tv_usec/1000000.0f) )/AMPDU_WIN,result); */
       return result;
}

void client_stats::getAMPDU_stats(){
        AMPDU_map();
        /* Debug("client_stats: getAMPDU_stats") */

        if(mAMPDU_map.empty()){
                mAver = 0.0;
                mSTD = 0.0;
                Debug("***EMPTY AMPDU_map list***");
                return ;
        }
        int pre_t;
        vector<int> v;
        vector<double> fv;
        for(map<packet*,int>::iterator it=mAMPDU_map.begin(); it != mAMPDU_map.end();++it){
                if( it ==  mAMPDU_map.begin()){
                        v.clear();
                        v.push_back(it->second);
                        pre_t = time_index(it->first->time);
                }
                else{
                        Debug("getAMPDU_stats: rounded time "<<pre_t<<"\t"<< time_index(it->first->time)<<"\t"<<(it->first )->time );
                        if( pre_t !=  time_index(it->first->time)){
                                Debug("getAMPDU_stats: one segment "<<average(v));
                                fv.push_back(average(v));
                                v.clear();
                                v.push_back(it->second);
                                pre_t = time_index(it->first->time);
                        }
                        else{
                                Debug("getAMPDU_stats: sub segment "<<it->second);
                                v.push_back(it->second);
                        }
                }
        }
        fv.push_back(average(v));
        mAver=average(fv);
        mSTD=deviation(fv,mAver);
}

double client_stats::getAver(){
        return mAver;
}
double client_stats::getSTD(){
        return mSTD;
}
timeval client_stats::getUpTime(){
        return mClientUpTime;
}

void client_stats::setPool(AP_pool* p){
        mPool=p;
}
///////////////////////////////////////////////////////////////////////////
/////////////////////AP_stats/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

AP_stats::AP_stats(string mac):mAPUpTime(TIME_NONE),mSSID("unknown"){
        mMac_address = mac;
}

void AP_stats::add_packet(packet* p){
        /* Debug("AP_stats "<<p->dst<<"  ::Add Packet.") */
        /* Debug("AP_stats "<<p->dst<<" Num of Clients "<<mClients.size()); */
        if(mClients.find(p->dst) != mClients.end()){
                /* Debug("AP_stats "<<p->dst<<"Exist."); */
                ( mClients.find(p->dst)->second )->add_packet(p);
        }else {
                client_stats* c=new client_stats(p->dst);
                mClients[p->dst]=c;
                c->setPool(getPool());
                c->add_packet(p);
                Debug("AP_stats "<<p->dst<<" New client.");
        } 
        mAPUpTime = p->time;
        updateClients();
}
void AP_stats::updateClients(){
        for(map<string, client_stats*>::iterator it=mClients.begin();it!=mClients.end();++it){
                if(mPool->getUpTime() - it->second->getUpTime()>EXPIRE_TIME){
                        Debug("AP_stats updateClients "<<"pop out "<<mPool->getUpTime() <<"\t"<<it->second->getUpTime()<<"\t"<<mPool->getUpTime() - it->second->getUpTime()<<"\t"<<mClients.size());
                        mClients.erase(it);
                }
        }
}
void AP_stats::dump_report(){
        Debug("AP_stats: Report ")
        printf("%s(%s) ",mMac_address.c_str(),mSSID.c_str());
        for(map<string, client_stats*>::iterator it=mClients.begin();it!=mClients.end();++it){
                it->second->getAMPDU_stats();
                printf("%s:%.3f ",( it->first ).c_str(),it->second->getAver());
        }
        printf("\n");
}

timeval AP_stats::getUpTime(){
        Debug("AP_stats getUpTime"<<mAPUpTime);
        return mAPUpTime;
}
void AP_stats::setPool(AP_pool* p){
        mPool=p;
}

AP_pool* AP_stats::getPool(){
        return mPool;
}

void AP_stats::setSSID(string ssid){
        mSSID = ssid;
}
///////////////////////////////////////////////////////////////////////////
/////////////////////AP_pool/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

AP_pool::AP_pool(string name):mUpTime(TIME_NONE){
        mName = name;
}

void AP_pool::add_packet(packet* p){
        /* Debug("AP_pool "<<p->src<<"  ::Add Packet.") */
        if(mAPs.find(p->src) != mAPs.end()){
                mAPs.find(p->src)->second->add_packet(p);
        }
        else{
                AP_stats* a=new AP_stats(p->src);
                mAPs[p->src]=a;
                a->setPool(this);
                a->add_packet(p);
                Debug("AP_pool "<<p->dst<<" New AP.");
        }
        mUpTime = p->time;
        updateAPs();
}
void AP_pool::updateAPs(){
        for(map<string, AP_stats*>::iterator it=mAPs.begin();it!=mAPs.end();++it){
                if(mUpTime - it->second->getUpTime()>EXPIRE_TIME){
                        mAPs.erase(it);
                }
        }
}

timeval AP_pool::getUpTime(){
        return mUpTime;
}
AP_stats* AP_pool::find_AP(string mac){
        if(mAPs.find(mac) == mAPs.end()){
                return NULL;
        }
        else{
                return mAPs.find(mac)->second;
        }
}

void AP_pool::UpdateSSID(string addr, string ssid){
        AP_stats* p = find_AP(addr);
        if(p){
                p->setSSID(ssid);
        }
}
void AP_pool::report(){
        for(map<string, AP_stats*>::iterator it=mAPs.begin();it!=mAPs.end();++it){
                it->second->dump_report();
        }
}
