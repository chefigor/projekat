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
    uint16_t p = atoi(argv[1]);
    int c = atoi(argv[2]);
    Server s(p, c);
    s.Start();
}