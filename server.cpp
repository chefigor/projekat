#include "server.h"

Server::Server()
{
    port = 54545;
    connections = 10;
}
Server::Server(uint16_t port, uint8_t connections)
{
    this->port = port;
    this->connections = connections;
}

std::string Server::CommandParser(std::string str)
{

    std::string delimiter = " ";
    size_t pos = 0;
    std::string token;
    std::vector<std::string> args;

    while ((pos = str.find(delimiter)) != std::string::npos)
    {
        token = str.substr(0, pos);
        args.push_back(token);
        str.erase(0, pos + delimiter.length());
    }
    args.push_back(str);

    if (args[0] == "get")
    {
        std::string v;
        if ((v = this->Get(args[1])) != "")
        {
            return v;
        }
        else
        {
            return "(nil)";
        }
    }
    else if (args[0] == "set")
    {
        std::string first = args[1];
        std::string second = args[2];
        this->Set(first, second);
        return "OK";
    }
    else if (args[0] == "del")
    {
        this->Del(args[1]);
        return "OK";
    }

    return "";
}
void Server::Set(std::string key, std::string value)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    map[key] = value;
}
std::string Server::Get(std::string key)
{
    std::shared_lock<std::shared_mutex> lock(mutex);
    if (auto it = map.find(key); it != map.end())
    {
        return it->second;
    }
    return "";
}
void Server::Del(std::string key)
{
    std::unique_lock<std::shared_mutex> lock(mutex);
    map.erase(key);
}

void Server::Connect(int sfd)
{

    size_t bsize = 1024;
    char buffer[bsize];
    memset(buffer, 0, bsize);

    while (true)
    {
        memset(buffer, 0, bsize);
        if (recv(sfd, buffer, bsize, 0) <= 0)
        {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }
        std::cout << "Client:" << buffer << std::endl;
        std::string cstring(buffer);
        if (strcmp(buffer, "quit\0") == 0)
        {
            std::string qmsg("Quitting...");
            if (send(sfd, qmsg.c_str(), qmsg.length() + 1, 0) == -1)
            {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }
            close(sfd);

            return;
        }

        std::string msg = CommandParser(cstring);
        if (msg == "")
        {
            std::string qmsg("Greska");
            if (send(sfd, qmsg.c_str(), qmsg.length() + 1, 0) == -1)
            {
                std::cout << "Greska pri slanju" << std::endl;
                return;
            }
        }
        if (send(sfd, msg.c_str(), msg.length() + 1, 0) == -1)
        {
            std::cout << "Greska pri slanju" << std::endl;
            return;
        }
    }
}

void Server::Start()
{
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

    if ((status = getaddrinfo(NULL, s, &hints, &res) != 0))
    {
        std::cout << "getaddrinfo:"
                  << gai_strerror(status) << std::endl;
        return;
    }

    for (p = res; p != nullptr; p = p->ai_next)
    {
        if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
        {
            std::cout << "Greska kod socket f-je:"
                      << strerror(errno) << std::endl;
            continue;
        }

        if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1)
        {
            close(sockfd);
            std::cout << "Greska kod bind f-je:"
                      << strerror(errno) << std::endl;
            continue;
        }
        break;
    }

    freeaddrinfo(res);

    if (listen(sockfd, connections) == -1)
    {
        std::cout << "Greska kod listen f-je:"
                  << strerror(errno) << std::endl;
        return;
    }

    while (true)
    {
        addr_size = sizeof their_addr;
        if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size)) == -1)
        {
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