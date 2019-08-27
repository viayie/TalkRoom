#include "../inc/includes.h"

int sockfd;

int main()
{
    int ret = SIGN();
    if(ret == 2){//未成功登入且请求退出
        Close(sockfd);
    }

    return 0;
}
