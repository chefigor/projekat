#include <stdio.h>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <cstring>

int main(int argc, char *argv[])
{
    struct addrinfo hints, *res, *p;
    int status;
    char ipstr[INET_ADDRSTRLEN];
    int sockfd;

    if (argc != 2)
    {
        fprintf(stderr, "Potrebna je  adresa servera\n");
        return -1;
    }

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(argv[1], "54545", &hints, &res)) != 0)
    {
        fprintf(stderr, "getaddrinfo:%s\n", gai_strerror(status));

        return -1;
    }

    if (res == NULL)
    {
        printf("Error!");
    }
    if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
    {
        printf("Greska kod socket f-je %s\n", strerror(errno));

        return -1;
    }

    if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1)
    {
        printf("Greska kod connect f-je %s\n", strerror(errno));
        return -1;
    }
    freeaddrinfo(res);

    while (true)
    {
        std::string m;
        getline(std::cin, m);
        if (send(sockfd, m.c_str(), m.length(), 0) < 1)
        {
            printf("Greska pri slanju : %s\n", strerror(errno));
            return -1;
        }

        size_t bsize = 1024;
        char buffer[bsize];
        //memset(buffer, 0, bsize);
        if (recv(sockfd, buffer, bsize, 0) <= 0)
        {
            printf("Greska pri primanju podataka : %s\n", strerror(errno));
            return -1;
        }
        printf("Server: %s\n", buffer);
    }
    close(sockfd);
    return 0;
}