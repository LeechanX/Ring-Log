#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "rlog.h"

struct echo_ctl
{
    int fd;
    int to_reply;
};

int main(int argc, char const *argv[])
{
    LOG_INIT("log", "newlog-newlog", 3);
    int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);

    struct sockaddr_in servaddr, cliaddr;
    socklen_t cli_len;
    bzero(&servaddr, sizeof (servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(12356);
    inet_aton("127.0.0.1", &servaddr.sin_addr);
    bind(sockfd, (struct sockaddr* )&servaddr, sizeof(servaddr));

    listen(sockfd, 10000);

    struct epoll_event events[100];
    int efd = epoll_create1(0);

    struct epoll_event ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    epoll_ctl(efd, EPOLL_CTL_ADD, sockfd, &ev);

    char buff[100];

    while (true)
    {
        int nfds = epoll_wait(efd, events, 10, 10);
        for (int i = 0;i < nfds; ++i)
        {
            if (events[i].data.fd == sockfd)
            {
                //connection is coming
                int connfd;
                while (true)
                {
                    connfd = accept(sockfd, (struct sockaddr* )&cliaddr, &cli_len);
                    if (connfd == -1)
                    {
                        if (errno == EINTR)
                            continue;
                        break;
                    }

                    int flag = fcntl(connfd, F_GETFL, 0);
                    fcntl(connfd, F_SETFL, O_NONBLOCK | flag);
                    struct epoll_event cev;

                    echo_ctl *ctl = new echo_ctl();
                    ctl->fd = connfd;
                    ctl->to_reply = 0;
                    cev.data.ptr = ctl;
                    cev.events = EPOLLIN;
                    epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &cev);
                }
            }
            else if (events[i].events & EPOLLIN)
            {
                echo_ctl *ctl = (echo_ctl*)events[i].data.ptr;
                int connfd = ctl->fd;
                int rd = recv(connfd, buff, 1000, 0);
                while (rd == -1)
                {
                    if (errno == EINTR)
                    {
                        rd = recv(connfd, buff, 1, 0);
                    }
                    else
                    {
                        delete ctl;
                        close(connfd);
                        break;
                    }
                }
                if (rd == 0)
                {
                    delete ctl;
                    close(connfd);
                }
                if (rd > 0)
                {
                    struct epoll_event cev;
                    ctl->to_reply += rd;
                    cev.data.ptr = ctl;
                    cev.events = EPOLLOUT | EPOLLIN;
                    epoll_ctl(efd, EPOLL_CTL_MOD, connfd, &cev);
		    for (int i = 0;i < rd; ++i)
                    {
                        LOG_ERROR("i receive data: %c", buff[i]);
                    }
                }
            }
            else if (events[i].events & EPOLLOUT)
            {
                echo_ctl* ctl = (echo_ctl*)events[i].data.ptr;
                int connfd = ctl->fd;
                int wr, wtn = 0;
                while (wtn < ctl->to_reply)
                {
                    char buff[ctl->to_reply - wtn];
                    memset(buff, 'X', ctl->to_reply - wtn);
                    wr = send(connfd, buff, ctl->to_reply - wtn, 0);
                    if (wr == -1)
                    {
                        if (errno == EINTR)
                            continue;
                        else if (errno == EAGAIN)
                        {
                            ctl->to_reply = ctl->to_reply - wtn;
                            struct epoll_event cev;
                            cev.data.ptr = ctl;
                            cev.events = EPOLLIN;
                            epoll_ctl(efd, EPOLL_CTL_MOD, connfd, &cev);
                            break;
                        }
                        else
                        {
                            delete ctl;
                            close(connfd);
                            break;
                        }
                    }
                    wtn += wr;
                }
                if (wtn == ctl->to_reply)
                {
                    ctl->to_reply = 0;
                    struct epoll_event cev;
                    cev.data.ptr = ctl;
                    cev.events = EPOLLIN;
                    epoll_ctl(efd, EPOLL_CTL_MOD, connfd, &cev);
                }
            }
        }
    }
    close(efd);
    close(sockfd);
    return 0;
}
