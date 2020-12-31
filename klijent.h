
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <openssl/evp.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <regex>
#include <string>
#include <vector>
class Klijent {
public:
    void InteractiveRun(std::string, std::string);
    void RunRegular(std::string, std::string, std::string);
    // bool send(int sfd);
    // bool recv();
private:
    int SendAll(int, std::vector<char> &, int);
    // void DigestMessage(const unsigned char *, size_t, unsigned char **,
    //                    unsigned int *);
    void DigestMessage(const std::vector<char>, std::string &);
};