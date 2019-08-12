# http_download
基于http协议的局域网下载工具
基础代码是httplib.h
httplib.h实现了http服务的底层细节
服务端：P2Pserver.hpp
实现了服务端的功能：
主机配对响应
共享文件列表获取
共享文件下载
客户端:P2Pclient.hpp
1. 主机配对请求
2. 在线主机列表的打印和选择获取在线主机共享文件列表
3. 共享列表打印和下载文件选择
