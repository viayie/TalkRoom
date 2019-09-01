#include "../inc/sign.h"
#include "../inc/wrap.h"
#include "../inc/conn.h"
#include "../inc/func.h"
#include <pthread.h>

int sockfd;
char nowname[20] = {0};//保存本次登录的用户名
pthread_t thread[2];

int main()
{
    /* 登录 */
    int ret = SIGN();
    if(ret == 2){//未成功登入且请求退出
        Close(sockfd);
        return 1;
    }

    /* 读写分离 */
    pthread_create(&thread[0], NULL, (void *)myread, NULL);
    pthread_detach(thread[0]);     
    pthread_create(&thread[1], NULL, (void *)mywrite, NULL);
    pthread_detach(thread[1]);

    pthread_exit(NULL);

    return 0;
}
