#pragma once
#include<iostream>
#include<string>
#include<boost/filesystem.hpp>
#include<vector>
#include<stdio.h>
#include<unistd.h>
#include<ifaddrs.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<boost/algorithm/string.hpp>
#include<sys/wait.h>
#include"httplib.h"
//安全性封装
class OnlineHost{
private:
    std::vector<std::string> _host_list;
    std::mutex _mutex;
public:
    bool AddHost(const std::string &host) {
        _mutex.lock();
        _host_list.push_back(host);
        _mutex.unlock();
        return true;

    }
    std::string GetHostById(const int idx){
        std::string host;
        _mutex.lock();
        host = _host_list[idx];
        _mutex.unlock();
        return host;

    }
    std::vector<std::string> GetHost() {
        std::vector<std::string> list;
        _mutex.lock();
        for (auto i : _host_list) {
            list.push_back(i);

        }
        _mutex.unlock();
        return list;

    }

};
OnlineHost _host_list;
class P2Pclient
{
public:
    P2Pclient(const std::string &path): _download_path(path) {
        if(!boost::filesystem::exists(path)){
            boost::filesystem::create_directory(path);
        }
        if(_download_path.back()!='/'){
            _download_path.push_back('/');
        }
    }
    ~P2Pclient() {}
    bool GetHostList(std::vector<std::string> &list){
//        struct ifaddrs   
//        {   
//            struct ifaddrs  *ifa_next;    /* Next item in list */   
//            char            *ifa_name;    /* Name of interface */   
//            unsigned int     ifa_flags;   /* Flags from SIOCGIFFLAGS */   
//            struct sockaddr *ifa_addr;    /* Address of interface */   
//            struct sockaddr *ifa_netmask; /* Netmask of interface */   
//            union   
//            {   
//                struct sockaddr *ifu_broadaddr; /* Broadcast address of interface */   
//                struct sockaddr *ifu_dstaddr; /* Point-to-point destination address */   
//            } ifa_ifu;   
//#define              ifa_broadaddr ifa_ifu.ifu_broadaddr   
//#define              ifa_dstaddr   ifa_ifu.ifu_dstaddr   
//            void            *ifa_data;    /* Address-specific data */   
//        };   
//
       
        struct ifaddrs *addrs;
        getifaddrs(&addrs);//用链表返回网络接口信息
        while(addrs){//如果一台主机有多块网卡，则可以全部遍历
            //排除lo信息
            if(strcmp(addrs->ifa_name,"lo")==0){
                addrs=addrs->ifa_next;
            }
            //获取ip地址和子网掩码
            sockaddr_in *addr=(sockaddr_in*)addrs->ifa_addr;
            sockaddr_in *mask=(sockaddr_in*)addrs->ifa_netmask;
            if(addr->sin_family!=AF_INET){
                addrs=addrs->ifa_next;
                continue;
            }
            //地址转化
            uint32_t net=ntohl((addr->sin_addr.s_addr)&(mask->sin_addr.s_addr));
            int host=ntohl(~mask->sin_addr.s_addr);
            //获取局域网中所有和本机处于同一网段的主机地址
            for(int i=2;i<host-1;i++){
              //  std::cout<<"i="<<i<<std::endl;
                struct in_addr ip;
                ip.s_addr=htonl(net+i);
                list.push_back(inet_ntoa(ip));
            }
            addrs=addrs->ifa_next;
        }
        return true;
    }
    //static void pair_host(std::string host){
    static void pair_host(std::string host){
        //创建socket发起一次配对请求
        httplib::Client client(host.c_str(),9000);
        auto rsp=client.Get("/hostpair");
        //对返回的状态信息进行判断，返回正确加入在线主机列表
        if(rsp&&rsp->status==200){
            _host_list.AddHost(host);
        }else{
            std::cout<<"host:["<<host<<"] pairhost failed\n";
        }


    }
    bool HostPair(std::vector<std::string> &list){
        //for(auto i:list){
        //    pair_host(i);
        //}
        //std::cerr<<"----------------Pair over\n";
        //return true;
        //创建线程来进行主机配对
        std::vector<std::thread> thr_list;
        for (auto i : list) {
            thr_list.push_back(std::thread(pair_host, i));

        }
        for (int i = 0; i < thr_list.size(); i++) {
            thr_list[i].join();

        }
        std::cerr << "-----------------thread pair over\n";
        return true;
    }
    //输出配对成功的主机信息
    void PrintHost(){
        std::vector<std::string> list;
        list=_host_list.GetHost();
        for(int i=0;i<list.size();++i){
            std::cout<<i<<".["<<list[i]<<"]"<<std::endl;
        }
        SelectHost();
    }
    void SelectHost(){
        std::cout<<"选择想要看看的主机：";
        fflush(stdout);
        std::cin>>_host_idx;
        GetFileList(_host_list.GetHostById(_host_idx));
    }
    //请求文件列表
    bool GetFileList(const std::string &host_addr){
        httplib::Client client(host_addr.c_str(),9000);
        auto rsp=client.Get("/list");
        if(rsp&&rsp->status==200){
            boost::split(_file_list,rsp->body,boost::is_any_of("\n"));
        }else{
            std::cerr<<"host:["<<host_addr<<"] get file list failed\n";
        }
        return true;
    }
    //输出文件列表
    void PrintFile(){
        for(int i=0;i<_file_list.size()-1;i++){
            std::cout<<i<<".["<<_file_list[i]<<"]\n";
        }
        SelectFile();
    }
    //选择下载的文件
    bool SelectFile(){
        std::cout<<"选择想要下载的文件id:";
        fflush(stdout);
        std::cin>>_file_idx;
        if(_file_idx>_file_list.size()){//判断是否越界
            return false;
        }
        DownLoadFile(_file_list[_file_idx]);
        return true;
    }
    //下载文件--整个文件下载
   // bool DownLoadFile(std::string &filename){
   //     int64_t fsize=0;
   //     std::string host=_host_list.GetHostById(_host_idx);//选定的主机
   //     httplib::Client client(host.c_str(),9000);
   //     auto rsp=client.Head(filename.c_str());//选择的文件名
   //     //返回的状态信息无异常，从正文中获取文件数据
   //     if(rsp&&rsp->status==200){
   //        // std::string len;
   //        // len=rsp->get_header_value("Content-Length");
   //        // std::stringstream tmp;
   //        // tmp<<len;
   //        // tmp>>fsize;
   //         boost::filesystem::path path(filename);
   //              std::string file=path.filename().string();
   //              std::string realpath=_download_path+file;
   //              std::fstream fs(realpath,std::ios::binary|std::ios::out);
   //              if(!fs.is_open()){
   //                  std::cerr<<"open file["<<realpath<<"] failed\n";
   //                  return false;
   //              }
   //              fs.seekp(0,std::ios::beg);
   //              fs.write(&rsp->body[0],rsp->body.size());
   //              if(!fs.good()){
   //                  std::cerr<<"write file:["<<realpath<<"] failed\n";
   //                  return false;
   //              }
   //             fs.close();
   //   }
   //     else{
   //         std::cerr<<"download file failed!\n";
   //         return false;
   //     }
   //     return true;
   // }
   // 下载文件--分块下载
    bool DownLoadFile(std::string &filename) {
        int64_t fsize = 0;
        std::string host = _host_list.GetHostById(_host_idx);
        httplib::Client client(host.c_str(), 9000);
        auto rsp = client.Head(filename.c_str());
        if (rsp && rsp->status == 200) {
            std::string len;
            len = rsp->get_header_value("Content-Length");
            std::stringstream tmp;
            tmp << len;
            tmp >> fsize;

        }
        int64_t range_size = 1024;
        int count = fsize / range_size;
        //2049 / 1024 = 2   0,      1,          2  
        //            //0~2048:           0~1023  1024~2047   2048~2048
        for (int i = 0 ; i <= count; i++) {
            int64_t range_start = i * range_size;
            int64_t range_end = ((i+1) * range_size) - 1;
            if (i == count) {
                range_end = fsize - 1;
            }
            int64_t range_len = range_end - range_start + 1;
        //采用进程来进行分块下载--线程的常规用法无法直接传送多个参数
            int pid = fork();
            if (pid == 0) {
                RangeFileDownLoad(host, filename, range_start,
                                  range_len);
                exit(0);
            }
        }
        for (int i = 0; i <= count; i++) {
            wait(NULL);
        }
        std::cerr << "file "<<filename<<" download success\n";
        return true;
    }

    bool RangeFileDownLoad(std::string &host, 
                           std::string &name, int64_t start, int64_t len) {
        httplib::Headers hdrs;
        int64_t range_start = start;
        int64_t range_end = len + start - 1;
        int64_t range_len = len;
        std::stringstream tmp;
        tmp << "bytes="<<range_start<<"-"<<range_end;
        std::string range = tmp.str();
        hdrs.insert(std::make_pair("Range",range.c_str()));
        httplib::Client client(host.c_str(), 9000);
        auto rsp = client.Get(name.c_str(), hdrs);
        if (rsp && rsp->status == 200) {
            boost::filesystem::path path(name);
            std::string file = path.filename().string();
            std::string realpath = _download_path + file;
            std::fstream fs(realpath, 
                            std::ios::binary | std::ios::out);
            if (!fs.is_open()) {
                std::cerr << "open file:["<<realpath<<"] failed\n";
                return false;

            }
            fs.seekp(range_start, std::ios::beg);
            fs.write(&rsp->body[0], rsp->body.size());
            if (!fs.good()) {
                std::cerr << "write file:["<<realpath<<"] failed\n";
                return false;

            }
            fs.close();

        }else {
            std::cerr << "download file failed!\n";
            return false;

        }

        return true;

    }

    int DoFace() {
        std::cout << "1. 附近主机配对\n";
        std::cout << "2. 显示附近在线主机\n";
        std::cout << "3. 显示所有文件名称\n";
        std::cout << "0. 退出\n";
        int choose;
        std::cout << "please input:";
        fflush(stdout);
        std::cin >> choose;
        if (choose > 3 || choose < 0) {
            std::cout << "choose error\n";
            exit(0);

        }
        if (choose == 0) {
            exit(0);

        }
        return choose;

    }
    bool Start(){
        while(1){
            std::cout<<"-------------"<<std::endl;
            int choose=DoFace();
            std::vector<std::string> list;
            switch(choose){
            case 1:
                    GetHostList(list);
            //std::cout<<"abc-------------"<<std::endl;
                    HostPair(list);
                    break;
            case 2:
                    PrintHost();
                    break;
            case 3:
                    PrintFile();
                    break;
            default:
                    exit(0);
            }
        }
        return true;
    }
private:
    int _host_idx;//主机下标
    int _file_idx;//文件下标
    std::vector<std::string> _file_list;//文件列表
//    std::vector<std::string> _host_list;
    std::string _download_path;//下载地址
};

