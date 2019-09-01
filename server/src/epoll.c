#include "../inc/epoll.h"

/*创建 socket, 初始化lfd，将lfd挂到红黑树上 */
void initlistensocket(int efd, short port)
{
    /*创建监听套接字*/
    int lfd = Socket(AF_INET, SOCK_STREAM, 0);

    /*修改lfd为非阻塞*/
    int flag = 0;
    if ((flag = fcntl(lfd, F_SETFL, O_NONBLOCK)) < 0) {
        printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
        return;
    }

    /*端口复用*/
    int opt = 1;
    setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    /*初始化lfd*/
    //void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg);
    //将lfd放置在将自定义结构体数组的最后
    //acceptconn()是lfd的回调函数
    //arg是回调函数acceptconn()的参数，指向lfd带的结构体中ptr指向的自定义结构体
    eventset(&g_events[MAX_EVENTS], lfd, acceptconn, &g_events[MAX_EVENTS]);

    /*将lfd挂在红黑树上，设定监听事件为读事件*/
    eventadd(efd, EPOLLIN, &g_events[MAX_EVENTS]);

    /*绑定*/
    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);
    Bind(lfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    
    /*监听*/
    Listen(lfd, 128);

    return;
}

/*将自定义结构体 myevent_s 成员变量 初始化*/
void eventset(struct myevent_s *ev, int fd, void (*call_back)(int, int, void *), void *arg)
{
    ev->fd = fd;
    ev->call_back = call_back;
    ev->events = 0;
    ev->arg = arg;
    ev->status = 0;
    ev->last_active = time(NULL);

    return;
}

/* 向 epoll监听的红黑树 添加一个 文件描述符 */
void eventadd(int efd, int events, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};
    epv.data.ptr = ev;
    epv.events = events; //EPOLLIN 或 EPOLLOUT
    ev->events = events; //EPOLLIN 或 EPOLLOUT

    int op;
    if(ev->status == 1){//已经在红黑树 g_efd 里
        op = EPOLL_CTL_MOD;//修改其属性
    }
    else{//不在红黑树里
        op = EPOLL_CTL_ADD;//将其加入红黑树 g_efd
        ev->status = 1;//并将status置1
    }

    if(epoll_ctl(efd, op, ev->fd, &epv) < 0){
        printf("event add failed fd[%d], events[%d]\n", ev->fd, events);
    }
    else{
        printf("event add OK fd[%d], events[%d]\n", ev->fd, events);
    }

    return;
}

/* 从epoll 监听的 红黑树中删除一个 文件描述符*/
void eventdel(int efd, struct myevent_s *ev)
{
    struct epoll_event epv = {0, {0}};

    if(ev->status == 0){//不在红黑树上
        return;
    }

    epv.data.ptr = ev;
    ev->status = 0;
    epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv);

    return;
}

/*lfd的回调函数：当有文件描述符就绪, epoll返回, 调用该函数 与客户端建立链接*/
void acceptconn(int lfd, int events, void *arg)
{
    struct sockaddr_in clieaddr;
    socklen_t clieaddrlen = sizeof(clieaddr);

    int cfd = Accept(lfd, (struct sockaddr*)&clieaddr, &clieaddrlen);

    int i;
    do{
        //从全局数组g_events中找一个空闲元素
        for(i=0; i<MAX_EVENTS; i++){
            if(g_events[i].status == 0){
                break;//得到空闲位置i后跳出for循环
            }
        }

        /*树上的节点数已满*/
        if(i == MAX_EVENTS){
            printf("%s:max connect limit[%d]\n", __func__, MAX_EVENTS);
            break;//跳出do{}while(0),相当于goto
        }

        /*修改cfd为非阻塞*/
        int flag = 0;
        if ((flag = fcntl(cfd, F_SETFL, O_NONBLOCK)) < 0) {
            printf("%s: fcntl nonblocking failed, %s\n", __func__, strerror(errno));
            break;
        }

        /* 给cfd设置一个 myevent_s 结构体, 回调函数 设置为 dataprocess */
        eventset(&g_events[i], cfd, dataprocess, &g_events[i]);
        eventadd(g_efd, EPOLLIN|EPOLLET, &g_events[i]);

    }while(0);

    char str[INET_ADDRSTRLEN];
    printf("new connect [%s:%d][time:%ld], pos[%d]\n",
        inet_ntop(AF_INET, &clieaddr.sin_addr.s_addr, str, sizeof(str)),
        ntohs(clieaddr.sin_port),
        g_events[i].last_active, 
        i);

    return;
}
