#ifndef __SIGN_H__
#define __SIGN_H__

#include <stdio.h>
#include <string.h>
#include <cjson/cJSON.h>
#include <unistd.h>

#include "wrap.h"
#include "conn.h"

extern int sockfd;
extern char nowname[20];

/* 登录菜单 */
void SIGN_menu(void);

/* 身份验证验证 */
int identity(char *name, char *password, char *cmd);

/* 登录操作 */
int signIN();

/* 注册操作 */
int signUP();

/* 处理登录注册操作 */
int SIGN(void);

#endif