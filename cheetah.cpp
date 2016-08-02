#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

using namespace std;

#define MAX_BUF_LEN (10240)
#define MAX_CLINET (4096)

const bool debug = false;

const int port = 80;
const char* host = "127.0.0.1";

const char *head = "GET /not-exists HTTP/1.0\r\nConnection: Keep-Alive\r\nHost: www.baidu.com\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n\r\n";
const int head_len = strlen(head);

typedef struct {
    int fd;
    int read_len;
    int total_len;
    char buf[MAX_BUF_LEN];
}Client;


Client list[4096];


void print_content(char *buf)
{
    cout << "=======================================" << endl;
    cout << buf << endl;
    cout << "======================================="  << endl;
}

void handle_send(Client *c)
{
    c->total_len = -1;
    c->read_len = 0;
    write(c->fd, head, head_len);
}


int handle_read(Client *c)
{
    int n = read(c->fd, c->buf + c->read_len, MAX_BUF_LEN - c->read_len);
    
    if (n <= 0)
    {
        return 0;
    }
    
    c->read_len += n;
    if ( c->total_len < 0)
    {
        // get header
        char *head_spilter = strstr(c->buf, "\r\n\r\n");
        if (NULL == head_spilter)
        {
            return 0;
        }
        
        // get total_len
        char *len_substr = strstr(c->buf, "Content-Length: ");
        if (NULL == len_substr)
        {
            // no content lenth in header try recv all data
            if (debug)
            {
                cout << " no content len in header" << endl;
                print_content(c->buf);
            }
            handle_send(c);
            
            return 1;
        }
        c->total_len = (int)atol(len_substr) + (head_spilter - c->buf) + 4;
    }
    
    if ( c->total_len > 0 && c->read_len >= c->total_len)
    {
        if (debug)
        {
            print_content(c->buf);
        }
        handle_send(c);
        return 1;
    }
    
    return 0;
}

int main(int argn, const char* argv[])
{
    int n = 0;
    int c = 0;

    struct sockaddr_in srv;
    socklen_t len = sizeof(srv);
    srv.sin_family = AF_INET;
    srv.sin_port = htons(port);
    srv.sin_addr.s_addr = inet_addr(host);

    n = atol(argv[1]);
    c = atol(argv[2]);

    cout << n << " " << c << endl;

    int iEpoll = epoll_create(4096);

    for (int i = 0; i < n; i++) {
        int sd = socket(AF_INET, SOCK_STREAM, 0);
        connect(sd, (struct sockaddr *)&srv, len);
        list[i].fd = sd;
        list[i].read_len = 0;
        list[i].total_len = -1;

        struct epoll_event stServerEvent; 
        stServerEvent.data.fd = i;     //设置与要处理的事件相关的文件描述符
        stServerEvent.events = EPOLLIN|EPOLLET;    //设置要处理的事件类型
        epoll_ctl(iEpoll, EPOLL_CTL_ADD, sd, &stServerEvent);//注册epoll事件
        write(sd, head, head_len);
    }

    cout << "init finish" << endl;

    struct epoll_event stServerEvent; 

    while(c > 0)
    {
        struct epoll_event events[1024];
        int iEventNum = epoll_wait(iEpoll, events, 1024, 1);
        if (debug)
        {
            cout << ">>>>>>>>>> epoll_wait " << iEventNum << " events " << endl;
        }
        
        for(int k = 0; k < iEventNum; k++)
        {
            int idx = events[k].data.fd;

            c -= handle_read(&(list[idx]));
            if (debug)
            {
                cout << ">>>>>>>>>>>>>>>>>> c = " << c << endl;
            }
        }
    }
    
    return 0;
}
