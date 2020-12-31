#include "server.h"

Server::Server() {
    port = 54545;
    connections = 10;
}
Server::Server(uint16_t port, uint8_t connections) {
    this->port = port;
    this->connections = connections;
}
bool Server::caseInsensitiveCompare(std::string& str1, std::string&& str2) {
    return ((str1.size() == str2.size()) &&
            std::equal(
                str1.begin(), str1.end(), str2.begin(), [](char& c1, char& c2) {
                    return (c1 == c2 || std::toupper(c1) == std::toupper(c2));
                }));
}

std::string Server::CommandParser(std::string& str) {
    std::vector<std::string> args;
    std::regex reg(
        R"(^(get|set|del|lpush|lpop|rpush|rpop|llen|lrange|hset|hget|hdel) +(\w+) *(\w+)? *(-?\w+)? *$)",  // ovako ne radi hset i lrange
        std::regex_constants::icase);
    std::smatch matches;
    std::regex regfile(
        R"(^(set|lpush|rpush|hset|) +(\w+) *(\w+)? *(\w+)? *)",
        std::regex_constants::icase);  // da li ce mozda nekada zadnje dve
                                       // capture grupe uhvatiti deo binarnog
                                       // podatka?
    if (std::regex_search(str, matches, reg))
        for (size_t i = 1; i < matches.size(); ++i) args.push_back(matches[i]);
    else if (std::regex_search(str, matches, regfile)) {
        for (size_t i = 1; i < matches.size(); ++i)
            if (matches[i] != "") args.push_back(matches[i]);
        std::string chash = args.back();
        std::string shash;
        std::string file(matches.suffix().str());

        DigestMessage(file, shash);
        if (chash != shash) return "Error!";
        if (caseInsensitiveCompare(args[0], "set"))
            return this->Set(args[1], file);
        if (caseInsensitiveCompare(args[0], "lpush"))
            return this->LPush(args[1], file);

        if (caseInsensitiveCompare(args[0], "rpush"))
            return this->RPush(args[1], file);

        if (caseInsensitiveCompare(args[0], "hset"))
            return this->HSet(args[1], args[2], file);

    } else
        return "";

    if (caseInsensitiveCompare(args[0], "get"))
        return this->Get(args[1]);
    else if (caseInsensitiveCompare(args[0], "set"))
        return this->Set(args[1], args[2]);
    else if (caseInsensitiveCompare(args[0], "del"))
        return this->Del(args[1]);
    else if (caseInsensitiveCompare(args[0], "lpush"))
        return this->LPush(args[1], args[2]);
    else if (caseInsensitiveCompare(args[0], "lpop"))
        return this->LPop(args[1]);
    else if (caseInsensitiveCompare(args[0], "rpush"))
        return this->RPush(args[1], args[2]);
    else if (caseInsensitiveCompare(args[0], "rpop"))
        return this->RPop(args[1]);
    else if (caseInsensitiveCompare(args[0], "llen"))
        return this->LLen(args[1]);
    else if (caseInsensitiveCompare(args[0], "lrange"))
        return this->LRange(args[1], args[2], args[3]);
    else if (caseInsensitiveCompare(args[0], "hset"))
        return this->HSet(args[1], args[2], args[3]);
    else if (caseInsensitiveCompare(args[0], "hget"))
        return this->HGet(args[1], args[2]);
    else if (caseInsensitiveCompare(args[0], "hdel"))
        return this->HDel(args[1], args[2]);
    else
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
    } else
        res = "(nil)";
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
    } else
        res = "(nil)";
    return res;
}
std::string Server::LLen(std::string key) {
    std::shared_lock<std::shared_mutex> lock(lmutex);
    if (auto it = lmap.find(key); it != lmap.end()) {
        return std::to_string(it->second.size());
    }
    return "(nil)";
}
std::string Server::LRange(std::string key, std::string sstart,
                           std::string send) {
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
        else if (start >= 0 && start < it->second.size() && end >= start &&
                 end < it->second.size())
            for (int i = start; i <= end; i++) res += it->second[i] + " ";
        else
            res += "Out of range";
    }
    return res;
}

std::string Server::HSet(std::string key, std::string field,
                         std::string value) {
    std::unique_lock<std::shared_mutex> lock(hmutex);
    // hmap[key].operator[](field) = value;
    hmap[key][field] = value;
    return "OK";
}
std::string Server::HDel(std::string key, std::string field) {
    std::unique_lock<std::shared_mutex> lock(hmutex);
    if (auto it = hmap.find(key); it != hmap.end()) {
        if (it->second.erase(field)) {
            if (it->second.size() == 0) hmap.erase(key);
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
std::string Server::Set(std::string key, std::string& value) {
    std::unique_lock<std::shared_mutex> lock(lmutex);
    map[key] = value;
    return "OK";
}
const std::string& Server::Get(std::string key) {
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (auto it = map.find(key); it != map.end()) {
        return it->second;
    }
    return "(nil)";
}
std::string Server::Del(std::string key) {
    std::unique_lock<std::shared_mutex> lock(mutex);
    if (map.erase(key)) return "OK";
    return "";
}

void Server::Connect(int sfd) {
    std::vector<char> recvbuff;
    while (true) {
        uint32_t size;

        if (recv(sfd, &size, sizeof(size), MSG_WAITALL) <= 0) {
            std::cout << "Greska pri primanju podataka ili zatvorna veza"
                      << std::endl;
            return;
        }
        size = ntohl(size);
        std::cout << "Client message length:" << size << std::endl;

        recvbuff.resize(size);
        if (recv(sfd, recvbuff.data(), size, MSG_WAITALL) <= 0) {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }

        std::string cstring(recvbuff.begin(), recvbuff.end());
        // std::cout << "Client message:" << cstring << std::endl;

        if (cstring == "quit") {
            std::string qmsg("Quitting...");
            size = htonl(qmsg.size());

            if (send(sfd, &size, sizeof(size), 0) < 1) {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }

            if (send(sfd, qmsg.data(), qmsg.size(), 0) == -1) {
                std::cout << "Greska pri slanju sendbuff" << std::endl;
                return;
            }

            close(sfd);
            return;
        }

        std::string msg = CommandParser(cstring);
        if (msg == "") {
            std::string qmsg("Greska");
            size = htonl(qmsg.size());

            if (send(sfd, &size, sizeof(size), 0) == -1) {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }
            std::vector<char> sendbuff(qmsg.begin(), qmsg.end());
            if (send(sfd, sendbuff.data(), sendbuff.size(), 0) == -1) {
                std::cout << "Greska pri slanju sendbuff" << std::endl;
                return;
            }
        } else {
            size = htonl(msg.size());

            if (send(sfd, &size, sizeof(size), 0) == -1) {
                std::cout << "Greska pri slanju buffsize" << std::endl;
                return;
            }
            std::vector<char> sendbuff(msg.begin(), msg.end());
            int len = sendbuff.size();
            if (SendAll(sfd, sendbuff, len) == -1) {
                std::cout << "Greska pri slanju poruke" << std::endl;
                std::cout << "Poslato je samo " << len << " bajta" << std::endl;
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
    const char* s = f;

    if ((status = getaddrinfo(NULL, s, &hints, &res) != 0)) {
        std::cout << "getaddrinfo:" << gai_strerror(status) << std::endl;
        return;
    }

    for (p = res; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(res->ai_family, res->ai_socktype,
                             res->ai_protocol)) == -1) {
            std::cout << "Greska kod socket f-je:" << strerror(errno)
                      << std::endl;
            continue;
        }

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
            close(sockfd);
            std::cout << "Greska kod bind f-je:" << strerror(errno)
                      << std::endl;
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (listen(sockfd, connections) == -1) {
        std::cout << "Greska kod listen f-je:" << strerror(errno) << std::endl;
        return;
    }

    while (true) {
        addr_size = sizeof their_addr;
        if ((new_fd = accept(sockfd, (struct sockaddr*)&their_addr,
                             &addr_size)) == -1) {
            std::cout << "Greska kod listen f-je:" << strerror(errno)
                      << std::endl;
            continue;
        }

        std::thread t(&Server::Connect, this, new_fd);
        t.detach();
    }
    close(sockfd);
    return;
}

int Server::SendAll(int sockfd, std::vector<char>& sendbuff, int& len) {
    int total = 0;
    int bytesLeft = len;
    int n;
    while (total < len) {
        n = send(sockfd, sendbuff.data() + total, bytesLeft, 0);
        if (n == -1) {
            break;
        }
        total += n;
        bytesLeft -= n;
    }
    len = total;
    return n == -1 ? -1 : 0;
}

void Server::DigestMessage(const std::string& unhashed, std::string& hashed) {
    EVP_MD_CTX* mdctx;

    if ((mdctx = EVP_MD_CTX_new()) == NULL) std::cout << "Greska!" << std::endl;

    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
        std::cout << "Greska! EVP_DigestInit_ex" << std::endl;

    if (1 != EVP_DigestUpdate(mdctx, unhashed.data(), unhashed.length()))
        std::cout << "Greska! EVP_DigestUpdate" << std::endl;

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    if (1 != EVP_DigestFinal_ex(mdctx, hash, &lengthOfHash))
        std::cout << "Greska! EVP_DigestFinal_ex" << std::endl;

    std::stringstream ss;
    for (unsigned int i = 0; i < lengthOfHash; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    hashed = ss.str();
    EVP_MD_CTX_free(mdctx);
}