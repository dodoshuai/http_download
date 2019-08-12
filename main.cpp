#include <iostream>
#include<thread>
#include"P2Pclient.hpp"
#include"P2Pserver.hpp"
void ServerRun(){
    P2Pserver server;
    server.Start();
}
int main()
{
    std::thread server_thr(ServerRun);
    server_thr.detach();
    P2Pclient client("./download");
    client.Start();
    return 0;
}

