#include "server.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Potreban je port servera i broj dozvoljenih konekcija" << std::endl;
        return -1;
    }
    int p = atoi(argv[1]);
    auto port = htons(p);
    int c = atoi(argv[2]);
    int connections = c;
    Server s(port, c);
    s.Start();
}