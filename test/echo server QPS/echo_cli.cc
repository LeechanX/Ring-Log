#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <strings.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char const *argv[])
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(12356);
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    int sendbuff = 104857600;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendbuff, sizeof(sendbuff)) == -1)
        printf("Error setsockopt");

    if (connect(sockfd, (struct sockaddr* )&servaddr, sizeof(servaddr)) == -1)
    {
        perror("connect()");
        exit(1);
    }
    unsigned cnt = 0;
    char buff[1000];
    long lst_time = time(NULL);
    while (true)
    {
        int wn = send(sockfd, "X", 1, 0);
        if (wn == -1)
            break;
        int rd = recv(sockfd, buff, 1, 0);
        if (rd == -1 || rd == 0)
            break;
        cnt += 1;
        long cur_time = time(NULL);
        if (cur_time - lst_time >= 1)
        {
            printf("cnt %u\n", cnt);
            cnt = 0;
            lst_time = cur_time;
        }
    }
    close(sockfd);
    return 0;
}