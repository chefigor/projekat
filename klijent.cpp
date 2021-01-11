#include "klijent.h"

void Klijent::DigestMessage(const std::vector<char> unhashed,
                            std::string& hashed) {
    EVP_MD_CTX* mdctx;

    if ((mdctx = EVP_MD_CTX_new()) == NULL) std::cout << "Greska!" << std::endl;

    if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL))
        std::cout << "Greska! EVP_DigestInit_ex" << std::endl;

    if (1 != EVP_DigestUpdate(mdctx, unhashed.data(), unhashed.size()))
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
int Klijent::SendAll(int sockfd, std::vector<char>& sendbuff, int len) {
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

    auto start = std::chrono::steady_clock::now();
    std::vector<char> recvbuff;
    while (true) {
        // send
        auto end = std::chrono::steady_clock::now();
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
                  << " Client:" << std::endl;
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
        end = std::chrono::steady_clock::now();
        recvbuff.push_back('\0');
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count()
                  << " Server:" << recvbuff.data() << std::endl;
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
    std::vector<char> filebuff;
    std::string hash;

    std::cout << sendbuff.capacity() << std::endl;

    size_t last = 0;
    size_t next = 0;
    std::string s;
    while ((next = m.find(" ", last)) != std::string::npos) {
        // std::cout << m.substr(last, next - last + 1) << std::endl;
        s = m.substr(last, next - last + 1);
        sendbuff.insert(sendbuff.end(), s.begin(), s.end());
        std::cout << sendbuff.capacity() << std::endl;

        last = next + 1;
    }
    std::string filename = m.substr(last);

    std::regex reg(R"(^(set|lpush|rpush|hset))");
    std::smatch matches;
    bool slanjevrednosti = false;
    if (std::regex_search(m, matches, reg)) slanjevrednosti = true;

    if (slanjevrednosti) {
        std::string line;
        std::ifstream is(filename,
                         std::ios::in | std::ios::binary | std::ios::ate);

        is.unsetf(std::ios::skipws);
        std::streampos fsize;
        char* memblock;

        if (is.is_open()) {
            int fsize = is.tellg();
            std::cout << filebuff.capacity() << std::endl;
            filebuff.reserve(fsize);
            std::cout << filebuff.capacity() << std::endl;

            is.seekg(0, std::ios::beg);
            // ma  sendbuff.reserve(fsize + sendbuff.capacity());
            filebuff.insert(filebuff.end(), std::istream_iterator<char>(is),
                            std::istream_iterator<char>());

            DigestMessage(filebuff, hash);

            std::cout << "Sha256=" << hash << std::endl;

            is.close();
            std::cout << fsize << std::endl;
            std::cout << filebuff.capacity() << std::endl;
        }
        uint32_t size = htonl(sendbuff.size() + filebuff.size() + hash.size());

        if (send(sockfd, &size, sizeof(size), 0) < 1) {
            std::cout << "Greska pri slanju duzine poruke" << std::endl;
            return;
        }

        int len = sendbuff.size();
        if (SendAll(sockfd, sendbuff, len) == -1) {
            std::cout << "Greska pri slanju poruke" << std::endl;
            std::cout << "Poslato je samo " << len << " bajta" << std::endl;
        }
        if (send(sockfd, hash.data(), hash.size(), 0) < 1) {
            std::cout << "Greska pri slanju duzine poruke" << std::endl;
            return;
        }
        len = filebuff.size();
        if (SendAll(sockfd, filebuff, filebuff.size()) == -1) {
            std::cout << "Greska pri slanju poruke" << std::endl;
            std::cout << "Poslato je samo " << len << " bajta" << std::endl;
        }
    } else {
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
    }

    // recv
    uint32_t size;
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

    if (!slanjevrednosti) {
        std::ofstream os(filename, std::ios::out | std::ios::binary);
        if (os.is_open()) {
            os.write(recvbuff.data(), recvbuff.size());
            os.close();
        }
        recvbuff.push_back('\0');

        std::cout << "Done!" << std::endl;
    } else {
        std::cout << "Server message: " << recvbuff.data() << std::endl;
    }

    close(sockfd);
    return;
}