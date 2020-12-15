
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

class Klijent {
   public:
    static void InteractiveRun(std::string, std::string);
    static void RunRegular(std::string, std::string,std::string);
};