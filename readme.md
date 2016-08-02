# Cheetah 

A light wight web performance test tool. Use epoll as basic event driver.

## edit

edit port host and head in source file for your own test.

    const bool debug = false;

    const int port = 80;
    const char* host = "127.0.0.1";

    const char *head = "GET /not-exists HTTP/1.0\r\nConnection: Keep-Alive\r\nHost: www.baidu.com\r\nUser-Agent: ApacheBench/2.3\r\nAccept: */*\r\n\r\n";

## make

g++ cheetah.cpp -o cheetah


## use

time ./cheetah [client-count] [total-count]

> time ./cheetah 1000 100000

