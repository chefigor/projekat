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
    std::unordered_map<std::string, std::deque<std::string>> lmap;
    std::unordered_map<std::string, std::unordered_map<std::string, std::string>> hmap;
    std::shared_mutex mutex;
    std::shared_mutex lmutex;
    std::shared_mutex hmutex;

private:
    void Connect(int sfd);
    std::string Get(std::string);
    std::string Set(std::string, std::string);
    std::string Del(std::string);
    std::string CommandParser(std::string);
    std::string LPush(std::string, std::string);
    std::string RPush(std::string, std::string);
    std::string LPop(std::string);
    std::string RPop(std::string);
    std::string LLen(std::string);
    std::string LRange(std::string, std::string, std::string);
    std::string HSet(std::string, std::string, std::string);
    std::string HDel(std::string, std::string);
    std::string HGet(std::string, std::string);

public:
    Server();
    Server(uint16_t, uint8_t);
    void Start();
};