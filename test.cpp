#include <iostream>
#include"P2Pserver.hpp"
//#include"httplib.h"
int main()
{
    //httplib::Server srv;
    //srv.Get("/",HelloWorld);
    //srv.listen("192.168.80.128",9000);
    //std::cout << "Hello world" << std::endl;
    P2Pserver abc;
    abc.Start();
    return 0;
}

