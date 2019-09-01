#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdio.h>
#include <stdlib.h>
#include <cjson/cJSON.h>
#include <string.h>
#include <mysql/mysql.h>

#include "wrap.h"
#include "epoll.h"

#define MAX_EVENTS  1024    //监听上限数

extern MYSQL* pConn;       // 连接对象
extern struct epoll_event events[MAX_EVENTS+1];//保存已经满足就绪事件的文件描述符数组

/* 数据处理 */
void dataprocess(int lfd, int events, void *arg);

/* 处理登录 */
char *signIN(struct myevent_s *ev);

/* 处理注册 */
char *signUP(struct myevent_s *ev);

/* 处理私聊 */
char *private_chat(int fd, const char *buf);

#endif