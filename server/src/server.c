#include "../inc/epoll.h"
#include "../inc/wrap.h"
#include "../inc/process.h"
#include <mysql/mysql.h>

MYSQL* pConn;       // 连接对象
struct epoll_event events[MAX_EVENTS+1];//保存已经满足就绪事件的文件描述符数组

int main(int argc, char *argv[])
{
    /* 数据库初始化 */
    pConn = mysql_init(NULL);
    if(!pConn){
        printf("pConn初始化失败\n");
        return -1; 
    }
    else{
        printf("pConn初始化成功\n");
    }
    pConn = mysql_real_connect(pConn, "localhost", "root", "viayie", "TalkRoom", 0, NULL, 0);
    if(!pConn){
        printf("pConn连接失败\n");
        return -1;
    }
    else{
        printf("pConn连接成功\n");
    }   

    /*使用默认端口 或 命令行参数传入用户指定端口*/
    unsigned short port = SERV_PORT;
    if(argc == 2){
        port = atoi(argv[1]);
    }

    /*创建红黑树，返回给全局g_efd*/
    g_efd = epoll_create(MAX_EVENTS + 1);
    if(g_efd <= 0){
        printf("create efd in %s err : %s\n", __func__, strerror(errno));
        return 1;
    }

    /*初始化监听socket*/
    initlistensocket(g_efd, port);

    printf("server running : port[%d]\n", port);

    /*变量定义*/
    int checkpos = 0, i;
    int nfd;

    while(1){
        /*开始监听*/
        //将满足事件的文件描述符加入到events数组中，1秒钟没有事件满足，返回0       
        nfd = epoll_wait(g_efd, events, MAX_EVENTS+1, 1000);
        if(nfd < 0){
            printf("epoll_wait err, exit...\n");
            break;
        }

        /*轮询满足监听事件的文件描述符，循环nfd次*/
        for(i=0; i<nfd; i++){
            /*使用自定义结构体myevent_s类型指针, 接收 联合体data的void *ptr成员*/
            struct myevent_s *ev = (struct myevent_s *)events[i].data.ptr;
            
            if((events[i].events & EPOLLIN) && (ev->events & EPOLLIN)){
                ev->call_back(ev->fd, events[i].events, ev->arg);
            }
        }
    }
    
    /* 退出前释放所有资源 */
    return 0;
}