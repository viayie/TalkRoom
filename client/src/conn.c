#include "../inc/conn.h"

/* 连接服务器 */
int connSERV()
{
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERV_IP, &servaddr.sin_addr.s_addr);
    servaddr.sin_port = htons(SERV_PORT);
    Connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    return 1;
}

/* 断开服务器 */
int disconnSERV()
{
    Close(sockfd);
}