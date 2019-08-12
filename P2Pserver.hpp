
#include<iostream>
#include<boost/filesystem.hpp>
#include"httplib.h"
#include<string>
//#define ROOT_PATH "/usr/share/list"
#define ROOT_PATH "root"
#define LIST_PATH "/list/"
namespace bf = boost::filesystem;
class P2Pserver
{
public:
    P2Pserver() {}
    ~P2Pserver() {}
    //返回状态信息
    static void GetHostPair(const httplib::Request &req,httplib::Response &rsp){
        rsp.status=200;
        rsp.set_header("Content-Length","0");
    }
    //获取文件列表信息
    static bool GetFileList(const httplib::Request &req,httplib::Response &rsp){
        std::string body;
        std::string realpath=ROOT_PATH+req.path;//文件地址
        if(!bf::exists(realpath)){//判断realpath目录是否存在
            bf::create_directory(realpath);//创建realpath目录
        }
        std::vector<std::string> list;
        //获取文件目录
        bf::directory_iterator item_begin(realpath);
        bf::directory_iterator item_end;
        for(;item_begin!=item_end;++item_begin){
            //如果是目录则跳过
            if(bf::is_directory(item_begin->status())){
                continue;
            }
            //否则加入list中
            std::string name=item_begin->path().filename().string();
            std::string file=LIST_PATH+name;  
            list.push_back(file);
        }
        //将文件信息放入正文中
        for(auto i:list){
            body +=i+"\n";    
        }
        //设置键值对信息
        rsp.set_content(body,"text/html");
        std::string len=std::to_string(body.size());
        rsp.set_header("Content-Length",len.c_str());
        rsp.status=200;
        return true;
    }
    //文件数据获取
    static void GetFileData(const httplib::Request &req,httplib::Response &rsp){
        std::string realpath=ROOT_PATH+req.path;//文件地址
        if(!bf::exists(realpath)){//判断文件地址是否存在
            std::cerr<<"file["<<realpath<<"] is not exists\n";
            rsp.status =404;
        }
        //如果请求方法为“HEAD”则返回文件长度
        int64_t fsize=bf::file_size(realpath);
        if(req.method=="HEAD"){
            rsp.set_header("Content-Length",std::to_string(fsize).c_str());
            rsp.status=200;
            return ;
        }
        std::ifstream file(realpath,std::ios::binary);
        if(!file.is_open()){//判断文件是否打开
            rsp.status=500;
            return ;
        }
        //进行文件定位，获取读取文件数据长度
        file.seekg(0,file.end);
        int length=file.tellg();
        //文件指针定位，读取相应长度的数据
        file.seekg(0,file.beg);
        file.read(&rsp.body[0],length);
        if(!file.good())
        {
            rsp.status=500;
            return ;
        }
        file.close();
        rsp.set_header("Content_Type","application/octet_stream");
        rsp.status=200;
        return ;
        
    }
    static void HelloWorld(const httplib::Request &req, httplib::Response &rsp) {
                    //req.path = /   realpath = root/
      //  std::string realpath = ROOT_PATH + req.path;
      //  if (!bf::exists(realpath)) {//判断地址是否存在
      //      rsp.status = 404;
      //      return;
      //  }
      //  std::vector<std::string> list;
       // get_file_list(realpath, LIST_PATH, list);
        std::string body;
        body = "<html><body><ol><h4>";
       // for (int i = 0; i < list.size(); i++) {
       //     body += "<li><a href='"+ list[i] +"'>"+ list[i] +"</a></li>";
       // }
        body += "Hello World!</h4></ol></body></html>";
        rsp.set_content(body, "text/html");
        rsp.status = 200;
    }
    bool Start() {
        if (!bf::exists(ROOT_PATH)) {
                            bf::create_directory(ROOT_PATH);
                                        
        }
                    // GET / HTTP/1.1
        srv.Get("/", HelloWorld);
        srv.Get("/hostpair", GetHostPair);
        srv.Get("/list", GetFileList);
        //srv.Get("/list", HelloWorld);
        srv.Get("/list/(.*)", GetFileData);
        srv.listen("192.168.80.128", 9000);
        return true;

    }
private:
    httplib::Server srv;
};
//int main(){
//    P2Pserver abc;
//    abc.Start();
//    return 0;
//}
