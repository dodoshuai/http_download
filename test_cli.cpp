#include <iostream>
#include "P2Pclient.hpp"
int main()
{
    P2Pclient c("/usr/share/download");
    c.Start();
    return 0;
}

