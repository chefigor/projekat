#include "server.h"

Server::Server() {
    port = 54545;
    connections = 10;
}
Server::Server(uint16_t port, uint8_t connections) {
    this->port = port;
    this->connections = connections;
}

std::string Server::CommandParser(std::string str) {
    std::vector<std::string> args;
    std::regex reg(R"(^(get|set|del|lpush|lpop|rpush|rpop|llen|lrange|hset|hget|hdel) +(\w+) *(\w+)? *(-?\w+)? *$)", std::regex_constants::icase);
    std::smatch matches;
    if (std::regex_search(str, matches, reg))
        for (size_t i = 1; i < matches.size(); ++i)
            args.push_back(matches[i]);
    else
        return "";

    if (args[0] == "get")
        return this->Get(args[1]);
    else if (args[0] == "set")
        return this->Set(args[1], args[2]);
    else if (args[0] == "del")
        return this->Del(args[1]);
    else if (args[0] == "lpush")
        return this->LPush(args[1], args[2]);
    else if (args[0] == "lpop")
        return this->LPop(args[1]);
    else if (args[0] == "rpush")
        return this->RPush(args[1], args[2]);
    else if (args[0] == "rpop")
        return this->RPop(args[1]);
    else if (args[0] == "llen")
        return this->LLen(args[1]);
    else if (args[0] == "lrange")
        return this->LRange(args[1], args[2], args[3]);
    else if (args[0] == "hset")
        return this->HSet(args[1], args[2], args[3]);
    else if (args[0] == "hget")
        return this->HGet(args[1], args[2]);
    else if (args[0] == "hdel")
        return this->HDel(args[1], args[2]);
    return "";
}

std::string Server::LPush(std::string key, std::string value) {
    std::unique_lock<std::shared_mutex> lock(lmutex);
    lmap[key].push_front(value);
    return "OK";
}
std::string Server::RPush(std::string key, std::string value) {
    std::unique_lock<std::shared_mutex> lock(lmutex);
    lmap[key].push_back(value);
    return "OK";
}
std::string Server::LPop(std::string key) {
    std::unique_lock<std::shared_mutex> lock(lmutex);
    std::string res;

    if (auto it = lmap.find(key); it != lmap.end()) {
        if (!it->second.empty()) {
            res = it->second.front();
            it->second.pop_front();
            if (it->second.empty()) lmap.erase(key);
        }
    }
    return res;
}
std::string Server::RPop(std::string key) {
    std::unique_lock<std::shared_mutex> lock(lmutex);
    std::string res;
    if (auto it = lmap.find(key); it != lmap.end()) {
        if (!it->second.empty()) {
            res = it->second.back();
            it->second.pop_back();
            if (it->second.empty()) lmap.erase(key);
        }
    }
    return res;
}
std::string Server::LLen(std::string key) {
    std::shared_lock<std::shared_mutex> lock(lmutex);
    if (auto it = lmap.find(key); it != lmap.end()) {
        return std::to_string(it->second.size());
    }
    return "(nil)";
}
std::string Server::LRange(std::string key, std::string sstart, std::string send) {
    std::shared_lock<std::shared_mutex> lock(lmutex);
    std::string res;
    size_t start, end;
    try {
        start = std::stoi(sstart);
        end = std::stoi(send);
    } catch (...) {
        return "Invalid input!";
    }
    if (auto it = lmap.find(key); it != lmap.end()) {
        if (start == 0 && end == -1)
            for (int i = 0; i < it->second.size(); i++)
                res += it->second[i] + " ";
        else if (start >= 0 && start < it->second.size() && end >= start && end < it->second.size())
            for (int i = start; i <= end; i++)
                res += it->second[i] + " ";
        else
            res += "Out of range";
    }
    return res;
}

std::string Server::HSet(std::string key, std::string field, std::string value) {
    std::unique_lock<std::shared_mutex> lock(hmutex);
    //hmap[key].operator[](field) = value;
    hmap[key][field] = value;
    return "OK";
}
std::string Server::HDel(std::string key, std::string field) {
    std::unique_lock<std::shared_mutex> lock(hmutex);
    if (auto it = hmap.find(key); it != hmap.end()) {
        if (it->second.erase(field)) {
            if (it->second.size() == 0)
                hmap.erase(key);
            return "OK";
        }
    }
    return "";
}
std::string Server::HGet(std::string key, std::string field) {
    std::shared_lock<std::shared_mutex> lock(hmutex);
    if (auto it = hmap.find(key); it != hmap.end())
        if (auto it2 = it->second.find(field); it2 != it->second.end())
            return it2->second;
    return "(nil)";
}
std::string Server::Set(std::string key, std::string value) {
    std::unique_lock<std::shared_mutex> lock(lmutex);
    map[key] = value;
    return "OK";
}
std::string Server::Get(std::string key) {
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (auto it = map.find(key); it != map.end()) {
        return it->second;
    }
    return "(nil)";
}
std::string Server::Del(std::string key) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (map.erase(key))
        return "OK";
    return "";
}

void Server::Connect(int sfd) {
    std::vector<char> buffsize(4);
    fill(buffsize.begin(), buffsize.end(), 0);
    while (true) {
        fill(buffsize.begin(), buffsize.end(), 0);
        if (recv(sfd, buffsize.data(), buffsize.size(), 0) <= 0) {
            std::cout << "Greska pri primanju podataka ili zatvorna veza" << std::endl;
            return;
        }

        unsigned int size = (int(buffsize[3]) << 24) + (int(buffsize[2]) << 16) + (int(buffsize[1]) << 8) + buffsize[0];
        std::cout << "Client message length:" << size << std::endl;

        std::vector<char> recvbuff(size);
        if (recv(sfd, recvbuff.data(), recvbuff.size() + 1, 0) <= 0) {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }

        std::string cstring(recvbuff.begin(), recvbuff.end());
        std::cout << "Client message:" << cstring << std::endl;

        if (cstring == "quit") {
            std::string qmsg("Quitting...");
            size = qmsg.size();
            memcpy(buffsize.data(), &size, sizeof(size));
            if (send(sfd, buffsize.data(), buffsize.size(), 0) < 1) {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }

            std::vector<char> sendbuff(qmsg.begin(), qmsg.end());
            if (send(sfd, sendbuff.data(), sendbuff.size(), 0) == -1) {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }
            close(sfd);
            return;
        }
        std::string msg = CommandParser(cstring);
        if (msg == "") {
            std::string qmsg("Greska");
            size = qmsg.size();
            memcpy(buffsize.data(), &size, sizeof(size));
            if (send(sfd, buffsize.data(), buffsize.size(), 0) == -1) {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }
            std::vector<char> sendbuff(qmsg.begin(), qmsg.end());
            if (send(sfd, sendbuff.data(), sendbuff.size(), 0) == -1) {
                std::cout << "Greska pri slanju sendbuff" << std::endl;
                return;
            }
        } else {
            size = msg.size();
            std::vector<char> buffsize(sizeof(size));
            memcpy(buffsize.data(), &size, sizeof(size));

            if (send(sfd, buffsize.data(), buffsize.size(), 0) == -1) {
                std::cout << "Greska pri slanju buffsize" << std::endl;
                return;
            }
            std::vector<char> sendbuff(msg.begin(), msg.end());
            if (send(sfd, sendbuff.data(), sendbuff.size(), 0) == -1) {
                std::cout << "Greska pri slanju sendbuff" << std::endl;
                return;
            }
        }
    }
}

void Server::Start() {
    int status;
    struct addrinfo hints, *res, *p;
    int sockfd, new_fd;
    socklen_t addr_size;
    struct sockaddr_storage their_addr;
    struct sockaddr_in sin;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    char f[6];
    sprintf(f, "%u", port);
    const char *s = f;

    if ((status = getaddrinfo(NULL, s, &hints, &res) != 0)) {
        std::cout << "getaddrinfo:"
                  << gai_strerror(status) << std::endl;
        return;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
            std::cout << "Greska kod socket f-je:"
                      << strerror(errno) << std::endl;
            continue;
        }

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            close(sockfd);
            std::cout << "Greska kod bind f-je:"
                      << strerror(errno) << std::endl;
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (listen(sockfd, connections) == -1) {
        std::cout << "Greska kod listen f-je:"
                  << strerror(errno) << std::endl;
        return;
    }

    while (true) {
        addr_size = sizeof their_addr;
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1) {
            std::cout << "Greska kod listen f-je:"
                      << strerror(errno) << std::endl;
            continue;
        }

        std::thread t(&Server::Connect, this, new_fd);
        t.detach();
    }
    close(sockfd);
    return;
}