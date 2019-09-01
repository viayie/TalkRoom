#ifndef __FUNC_H__s
#define __FUNC_H__

#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <pthread.h>

#include "wrap.h"

#define BUFLEN      4096

extern int sockfd;
extern char nowname[20];
extern pthread_t thread[2];

/* 读线程 */
void *myread(void *arg);

/* 写线程 */
void *mywrite(void *arg);

/* 功能菜单界面 */
void FUNC_menu(void);

/* 私聊 */
int private_chat();

/* 功能模块 */
int FUNC();

#endif
