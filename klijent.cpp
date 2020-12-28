#include "klijent.h"

int Klijent::SendAll(int sockfd, std::vector<char>& sendbuff, int& len) {
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
void Klijent::InteractiveRun(std::string adr, std::string port) {
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN];
    int sockfd;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(adr.data(), port.data(), &hints, &res)) != 0) {
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

        if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
            std::cout << "Greska kod status f-je:" << strerror(errno)
                      << std::endl;
            continue;
        }
        break;
    }
    freeaddrinfo(res);

    std::vector<char> recvbuff;
    while (true) {
        // send
        std::string m;
        getline(std::cin, m);
        uint32_t size = htonl(m.size());

        if (send(sockfd, &size, sizeof(size), 0) < 1) {
            std::cout << "Greska pri slanju duzine poruke" << std::endl;
            return;
        }

        std::vector<char> sendbuff(m.begin(), m.end());
        int len = sendbuff.size();
        if (SendAll(sockfd, sendbuff, len) == -1) {
            std::cout << "Greska pri slanju poruke" << std::endl;
            std::cout << "Poslato je samo " << len << " bajta" << std::endl;
        }

        // recv

        if (recv(sockfd, &size, sizeof(size), MSG_WAITALL) <= 0) {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }

        size = ntohl(size);
        std::cout << "Server message length:" << size << std::endl;

        recvbuff.resize(size);

        if (recv(sockfd, recvbuff.data(), recvbuff.size(), MSG_WAITALL) <= 0) {
            std::cout << "Greska pri primanju podataka" << std::endl;
            return;
        }
        recvbuff.push_back('\0');

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

        if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1) {
            std::cout << "Greska kod status f-je:" << strerror(errno)
                      << std::endl;
            continue;
        }
        break;
    }
    freeaddrinfo(res);

    // send

    std::vector<char> sendbuff;

    size_t last = 0;
    size_t next = 0;
    std::string s;
    while ((next = m.find(" ", last)) != std::string::npos) {
        // std::cout << m.substr(last, next - last + 1) << std::endl;
        s = m.substr(last, next - last + 1);
        sendbuff.insert(sendbuff.end(), s.begin(), s.end());
        last = next + 1;
    }

    std::cout << s.max_size() << std::endl;
    if (m.substr(0, m.find(" ", 0)) == "set") {
        std::string filename = m.substr(last);

        std::string line;
        std::ifstream is(filename,
                         std::ios::in | std::ios::binary | std::ios::ate);

        std::streampos fsize;
        char* memblock;

        if (is.is_open()) {
            fsize = is.tellg();
            memblock = new char[fsize];
            is.seekg(0, std::ios::beg);
            is.read(memblock, fsize);
            is.close();
            std::cout << fsize << " strlen:" << strlen(memblock) << std::endl;
            sendbuff.insert(sendbuff.end(), memblock, memblock + fsize);
            delete[] memblock;
        }
    }

    uint32_t size = htonl(sendbuff.size());

    if (send(sockfd, &size, sizeof(size), 0) < 1) {
        std::cout << "Greska pri slanju duzine poruke" << std::endl;
        return;
    }

    int len = sendbuff.size();
    if (SendAll(sockfd, sendbuff, len) == -1) {
        std::cout << "Greska pri slanju poruke" << std::endl;
        std::cout << "Poslato je samo " << len << " bajta" << std::endl;
    }

    // recv

    if (recv(sockfd, &size, sizeof(size), MSG_WAITALL) <= 0) {
        std::cout << "Greska pri primanju podataka" << std::endl;
        return;
    }

    size = ntohl(size);
    std::cout << "Server message length:" << size << std::endl;

    std::vector<char> recvbuff(size);

    if (recv(sockfd, recvbuff.data(), recvbuff.size(), MSG_WAITALL) <= 0) {
        std::cout << "Greska pri primanju podataka" << std::endl;
        return;
    }

    if (m.substr(0, m.find(" ", 0)) == "get") {
        std::string filename = m.substr(last);

        std::ofstream os(filename, std::ios::out | std::ios::binary);
        if (os.is_open()) {
            os.write(recvbuff.data(), recvbuff.size());
            os.close();
        }
        recvbuff.push_back('\0');
    }
    // std::cout << "Server message:" << recvbuff.data() << std::endl;

    close(sockfd);
    return;
}