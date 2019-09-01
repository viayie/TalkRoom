#include "../inc/sign.h"

/* 登录菜单 */
void SIGN_menu(void)
{
    system("clear");
    printf("\t\t------------------------------\n\n");
    printf("\t\t---------->1.sign in----------\n");
    printf("\t\t---------->2.sign up----------\n");
    printf("\t\t---------->0.exit   ----------\n\n");
    printf("\t\t------------------------------\n");
}

/* 登录操作 */
int signIN()
{
    char name[10], password[10];
    printf("\nsign in---->enter your name and password:");
    scanf("%s %s", name, password);
    int ret = identity(name, password, "sign in");
    bzero(nowname, sizeof(nowname));
    strcpy(nowname, name);
    sleep(1);
    return ret;
}

/* 注册操作 */
int signUP()
{
    char name[10], password[10];
    printf("\nsign up---->enter your name and password:");
    scanf("%s %s", name, password);
    int ret = identity(name, password, "sign up");
    bzero(nowname, sizeof(nowname));
    strcpy(nowname, name);
    sleep(1);
    return ret;
}

/* 身份验证验证 */
int identity(char *name, char *password, char *cmd)
{
    cJSON *ident = cJSON_CreateObject();

	/*在ident对象中添加第一个子条目("name": name), 该条目为string类型*/
	if(cJSON_AddStringToObject(ident, "name", name) == NULL){
		fprintf(stderr, "cJSON_AddStringToObject failed\n");
		cJSON_Delete(ident);
		return 0;
	}

    /*在ident对象中添加第二个子条目("password": password), 该条目为string类型*/
	if(cJSON_AddStringToObject(ident, "password", password) == NULL){
		fprintf(stderr, "cJSON_AddStringToObject failed\n");
		cJSON_Delete(ident);
		return 0;
	}

    /*在ident对象中添加第三个子条目("cmd": cmd), 该条目为string类型*/
	if(cJSON_AddStringToObject(ident, "cmd", cmd) == NULL){
		fprintf(stderr, "cJSON_AddStringToObject failed\n");
		cJSON_Delete(ident);
		return 0;
	}

    /*将JSON对象写入string对象中*/
	char *string = cJSON_Print(ident);
	cJSON_Delete(ident);

    /* 写给服务器 */
    Write(sockfd, string, strlen(string));

    /* 阻塞等待服务器验证并应答 */
    char buf[BUFSIZ] = {0};
    Read(sockfd, buf, sizeof(buf));

    /* 判断验证结果 */
    if(strcmp(buf, "-verified-") == 0){//验证成功
        printf("\n\t\t-%s succeed!!!\n", cmd);
        return 1;
    }
    else{//验证失败
        printf("\n\t\t%s\n", buf);
        return 0;
    }
}

/* 处理登录注册操作 */
int SIGN(void)
{
    int opt;
    int flag = 0;

    if(connSERV()){
        printf("connected!!!\n");
    }
    else{
        printf("connect err\texiting...\n");
        return 1;
    }

    while(1){
        SIGN_menu();
        scanf("%d", &opt);
        switch(opt){
            case 1:
            {
                flag = signIN();//flag=0继续循环
                break;
            }
            case 2:
            {
                flag = signUP();//flag=0继续循环
                break;
            }
            case 0:
            {
                flag = 2;//稍后需要关闭sockfd退出
                break;            
            }
            default:
            {
                flag = 0;
                break;
            }
        }
        if(flag == 1 || flag == 2){
            break;
        }
    }

    return flag;
}
