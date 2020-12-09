#include <cstring>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <iostream>
#include <thread>
#include <cstdint>
#include <unordered_map>
#include <shared_mutex>
#include <mutex>
#include <string>
#include <regex>
class Server
{
private:
    uint16_t port;
    uint8_t connections;
    std::unordered_map<std::string, std::string> map;
    mutable std::shared_mutex mutex;
    void Connect(int sfd);
    std::string Get(std::string);
    void Set(std::string, std::string);
    void Del(std::string);
    std::string CommandParser(std::string);

public:
    Server();
    Server(uint16_t, uint8_t);
    void Start();
};