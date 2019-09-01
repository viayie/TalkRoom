#include "../inc/func.h"

static char g_cmd[50] = "-opt-";
static char g_flag = 1;


/* 读线程 */
void *myread(void *arg)
{
    char buf[BUFLEN];
    FUNC_menu();

    while(1){
        Read(sockfd, buf, sizeof(buf));
        cJSON *buf_json = cJSON_Parse(buf);
        const cJSON *cmd = cJSON_GetObjectItemCaseSensitive(buf_json, "cmd");

        if(strcmp(cmd->valuestring, "-opt_private_chat-")==0){
            g_flag = 0;
            system("clear");
            printf("Enter the other party's name: ");
            strcpy(g_cmd, "-othername_private_chat-");
            g_flag = 1;
        }
        else if(strcmp(cmd->valuestring, "-verified_private_chat-")==0){
            g_flag = 0;
            printf("verified_private_chat!!!\n");
            strcpy(g_cmd, "-private_chat-");
            sleep(1);
            system("clear");
            const cJSON *othername = cJSON_GetObjectItemCaseSensitive(buf_json, "othername");
            printf("\t\t***private_chat with %s***\n", othername->valuestring);
            g_flag = 1;
        }
        else if(strcmp(cmd->valuestring, "-err_private_chat-")==0){
            g_flag = 0;
            printf("***name notfound or offline***\n");
            sleep(2);
            FUNC_menu();
            strcpy(g_cmd, "-opt-");
            g_flag = 1;

        }
        else if(strcmp(cmd->valuestring, "-req_private_chat-")==0){
            g_flag = 0;
            system("clear");
            const cJSON *othername = cJSON_GetObjectItemCaseSensitive(buf_json, "othername");
            printf("\t\t***private_chat with %s***\n", othername->valuestring);
            strcpy(g_cmd, "-private_chat-");
            g_flag = 1;
        }
        else if(strcmp(cmd->valuestring, "-private_chat-")==0){
            const cJSON *content  = cJSON_GetObjectItemCaseSensitive(buf_json, "content");
            if(strcmp(content->valuestring, "bye")==0){
                g_flag = 0;
                FUNC_menu();
                strcpy(g_cmd, "-opt-");
                g_flag = 1;
            }
            else{
                const cJSON *othername  = cJSON_GetObjectItemCaseSensitive(buf_json, "othername");
                printf("%s : %s\n", othername->valuestring, content->valuestring);
            }
        }
        else if(strcmp(cmd->valuestring, "-EXIT-")==0){
            printf("exiting.....\n");
            sleep(1);
            pthread_cancel(thread[0]);
            pthread_cancel(thread[1]);
        }
    }
    
}

/* 写线程 */
void *mywrite(void *arg)
{
    char buf[BUFLEN];
    while(1){
        while(!g_flag);
        
        scanf("%s", buf);
        cJSON *packet = cJSON_CreateObject();
        cJSON_AddStringToObject(packet, "cmd", g_cmd);
        cJSON_AddStringToObject(packet, "content", buf);
	    char *string = cJSON_Print(packet);
	    cJSON_Delete(packet);
        Write(sockfd, string, strlen(string));
        usleep(1000);
    }
}

/* 功能菜单界面 */
void FUNC_menu(void)
{
    system("clear");
    printf("\t\t------------------------------------\n\n");
    printf("\t\t---------->1.private chat ----------\n");
    printf("\t\t---------->2.online friend----------\n");
    printf("\t\t---------->3.file transfer----------\n");
    printf("\t\t---------->0.exit         ----------\n\n");
    printf("\t\t------------------------------------\n");
}

/* 私聊 */
int private_chat()
{
    char buf[BUFLEN];

    while(1)
    {
        if(strcmp(g_cmd, "-private_chat_verified-") != 0){
            strcpy(g_cmd, "private_chat");

            printf("private_chat---->Enter the other party's name: ");
            scanf("%s", buf);

            cJSON *packets = cJSON_CreateObject();
            cJSON_AddStringToObject(packets, "cmd", g_cmd);
            cJSON_AddStringToObject(packets, "selfname", nowname);
            cJSON_AddStringToObject(packets, "othername", buf);
            char *string = cJSON_Print(packets);
            cJSON_Delete(packets);

            Write(sockfd, string, strlen(string));
            Read(sockfd, buf, sizeof(buf));

            if(strcmp(buf, "-verified-") == 0){//验证成功
                printf("\n\t\t-%s succeed!!!\n", g_cmd);
                strcpy(g_cmd, "private_chat_verified");
                // return 1;
            }
            else{//验证失败
                printf("\n\t\t%s\n", buf);
                return 0;
            }
        }
            /* 读写分离 */
        pthread_t thread[2];
        pthread_create(&thread[0], NULL, (void *)myread, NULL);
        pthread_detach(thread[0]);     
        pthread_create(&thread[1], NULL, (void *)mywrite, NULL);
        pthread_detach(thread[1]);
    }

}

/* 功能模块 */
int FUNC()
{
    int opt;
    int flag = 0;

    while(1){
        FUNC_menu();
        scanf("%d", &opt);
        switch(opt){
            case 1:
            {
                flag = private_chat();
                break;
            }
            case 2:
            {
                break;
            }
            case 3:
            {
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

