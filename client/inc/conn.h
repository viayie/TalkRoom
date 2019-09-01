#ifndef __CONN_H__
#define __CONN_H__

#include <stdio.h>
#include <unistd.h> 
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <errno.h>
#include <ctype.h>

#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 8888
#define SERV_IP "127.0.0.1"

extern int sockfd;

/* 连接服务器 */
int connSERV();

/* 断开服务器 */
int disconnSERV();

#endif