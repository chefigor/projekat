#include "klijent.h"

void Klijent::InteractiveRun(std::string adr, std::string port) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN];
    int sockfd;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(adr.data(), port.data(), &hints, &res)) != 0) {
        std::cout << "getaddrinfo:"
                  << gai_strerror(status) << std::endl;
        return;
    }

    if (res == NULL) {
        std::cout << "Error!" << std::endl;
    }
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        std::cout << "Greska kod socket f-je:"
                  << strerror(errno) << std::endl;

        return;
    }

    if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
        std::cout << "Greska kod status f-je:"
                  << strerror(errno) << std::endl;
        return;
    }
    freeaddrinfo(res);

    while (true) {
        //send
        std::string m;
        getline(std::cin, m);
        unsigned int size = m.size();
        std::vector<char> buffsize(sizeof(size));
        memcpy(buffsize.data(), &size, sizeof(size));
        if (send(sockfd, /* m.c_str()*/ buffsize.data(), buffsize.size(), 0) < 1) {
            std::cout << "Greska pri slanju" << std::endl;
            return;
        }

        std::vector<char> sendbuff(m.begin(), m.end());
        if (send(sockfd, sendbuff.data(), sendbuff.size(), 0) < 1) {
            std::cout << "Greska pri slanju" << std::endl;
            return;
        }

        //recv
        fill(buffsize.begin(), buffsize.end(), 0);

        if (recv(sockfd, buffsize.data(), buffsize.size(), 0) <= 0) {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }

        size = (int(buffsize[3]) << 24) + (int(buffsize[2]) << 16) + (int(buffsize[1]) << 8) + buffsize[0];
        std::cout << "Server message length:" << size << std::endl;

        std::vector<char> recvbuff(size);

        if (recv(sockfd, recvbuff.data(), recvbuff.size(), 0) <= 0) {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }

        std::cout << "Server message:" << recvbuff.data() << std::endl;
    }
    close(sockfd);
    return;
}

void Klijent::RunRegular(std::string adr, std::string port, std::string m) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN];
    int sockfd;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(adr.data(), port.data(), &hints, &res)) != 0) {
        std::cout << "getaddrinfo:"
                  << gai_strerror(status) << std::endl;
        return;
    }

    if (res == NULL) {
        std::cout << "Error!" << std::endl;
    }
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1) {
        std::cout << "Greska kod socket f-je:"
                  << strerror(errno) << std::endl;

        return;
    }

    if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
        std::cout << "Greska kod status f-je:"
                  << strerror(errno) << std::endl;
        return;
    }
    freeaddrinfo(res);

    //send

    unsigned int size = m.size();
    std::vector<char> buffsize(sizeof(size));
    memcpy(buffsize.data(), &size, sizeof(size));
    if (send(sockfd, /* m.c_str()*/ buffsize.data(), buffsize.size(), 0) < 1) {
        std::cout << "Greska pri slanju" << std::endl;
        return;
    }

    std::vector<char> sendbuff(m.begin(), m.end());
    if (send(sockfd, sendbuff.data(), sendbuff.size(), 0) < 1) {
        std::cout << "Greska pri slanju" << std::endl;
        return;
    }

    //recv
    fill(buffsize.begin(), buffsize.end(), 0);

    if (recv(sockfd, buffsize.data(), buffsize.size(), 0) <= 0) {
        std::cout << "Greska pri primanju podataka" << std::endl;
        return;
    }

    size = (int(buffsize[3]) << 24) + (int(buffsize[2]) << 16) + (int(buffsize[1]) << 8) + buffsize[0];
    std::cout << "Server message length:" << size << std::endl;

    std::vector<char> recvbuff(size);

    if (recv(sockfd, recvbuff.data(), recvbuff.size(), 0) <= 0) {
        std::cout << "Greska pri primanju podataka" << std::endl;
        return;
    }

    std::cout << "Server message:" << recvbuff.data() << std::endl;

    close(sockfd);
    return;
}