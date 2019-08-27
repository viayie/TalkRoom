#ifndef __INCLUDES_H__
#define __INCLUDES_H__

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cjson/cJSON.h>

#include "sign.h"
#include "wrap.h"

#define MAXLINE 80
#define SERV_PORT 8888
#define SERV_IP "127.0.0.1"

extern int sockfd;

#endif