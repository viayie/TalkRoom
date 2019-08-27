#ifndef __SIGN_H__
#define __SIGN_H__

#include "includes.h"

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

/* 连接服务器 */
int connSERV();

/* 断开服务器 */
int disconnSERV();

#endif