#include "../inc/process.h"

/* 数据处理 */
void dataprocess(int lfd, int events, void *arg)
{
    struct myevent_s *ev = (struct myevent_s *)arg;

    int len = Read(ev->fd, ev->buf, sizeof(ev->buf));
    ev->last_active = time(NULL);   //当前时间
    ev->len = len;
    ev->buf[len] = '\0';
    printf("client[%d] Before dataprocess:\n%s\n", ev->fd, ev->buf);

    cJSON *buf_json = cJSON_Parse(ev->buf);
    const cJSON *cmd = cJSON_GetObjectItemCaseSensitive(buf_json, "cmd");
    const cJSON *content = cJSON_GetObjectItemCaseSensitive(buf_json, "content");
    char *str = NULL;
    cJSON *packet = cJSON_CreateObject();

    if(strcmp(cmd->valuestring, "sign in")==0){
        str = signIN(ev);
        Write(ev->fd, str, strlen(str));
    }
    else if(strcmp(cmd->valuestring, "sign up")==0){
        str = signUP(ev);
        Write(ev->fd, str, strlen(str));
    }
    else if(strcmp(cmd->valuestring, "-opt-")==0){
        /*私聊功能*/
        if(strcmp(content->valuestring, "1")==0){
            printf("client[%d]->FUNC:private_chat\n", ev->fd);
            cJSON_AddStringToObject(packet, "cmd", "-opt_private_chat-");
            str = cJSON_Print(packet);
            Write(ev->fd, str, strlen(str));
        }
        /*客户端退出*/
        if(strcmp(content->valuestring, "0")==0){
            printf("Client[%d] closed!\n", ev->fd);
            cJSON_AddStringToObject(packet, "cmd", "-EXIT-");
            str = cJSON_Print(packet);
            Write(ev->fd, str, strlen(str));
            char string[1024] = {0};
            sprintf(string, "UPDATE `TalkRoom`.`talkroom` SET `fd`='-1', `online`='0' WHERE `name`='%s';", ev->nowname);
            mysql_real_query(pConn, string, (unsigned int)strlen(string));
            eventdel(ev->fd, ev);
            Close(ev->fd);
        }
    }
    else if(strcmp(cmd->valuestring, "-othername_private_chat-")==0){
        /* 在数据库中查询 */
        char string[1024] = {0};
        MYSQL_RES* pRes;    // 结果集
        sprintf(string, "select * from TalkRoom.talkroom where name='%s' and online='%d';", content->valuestring, 1);
        mysql_real_query(pConn, string, (unsigned int)strlen(string));
        pRes = mysql_store_result(pConn);
        if(pRes){
            if((int)mysql_num_rows(pRes) == 1){//找到一行
                MYSQL_ROW result_row = mysql_fetch_row(pRes);
                
                int otherfd = atoi(result_row[0]);
                printf("otherfd=%d\n", otherfd);
                sprintf(string, "UPDATE `TalkRoom`.`talkroom` SET `privatetalk_otherfd`='%d' WHERE `name`='%s';", ev->fd, content->valuestring);
                mysql_real_query(pConn, string, (unsigned int)strlen(string));
                sprintf(string, "UPDATE `TalkRoom`.`talkroom` SET `privatetalk_otherfd`='%d' WHERE `name`='%s';", otherfd, ev->nowname);
                mysql_real_query(pConn, string, (unsigned int)strlen(string));

                /*写给对方客户端*/
                cJSON *otherpacket = cJSON_CreateObject();
                cJSON_AddStringToObject(otherpacket, "cmd", "-req_private_chat-");
                cJSON_AddStringToObject(otherpacket, "othername", ev->nowname);
                char *otherstr = cJSON_Print(otherpacket);
                Write(otherfd, otherstr, strlen(otherstr));
                cJSON_Delete(otherpacket);

                cJSON_AddStringToObject(packet, "cmd", "-verified_private_chat-");   
                cJSON_AddStringToObject(packet, "othername", content->valuestring);               
            }
            else{
                cJSON_AddStringToObject(packet, "cmd", "-err_private_chat-");            
            }
        }
        str = cJSON_Print(packet);
        Write(ev->fd, str, strlen(str));       
    }
    else if(strcmp(cmd->valuestring, "-private_chat-")==0){
        /* 在数据库中查询 */
        char string[1024] = {0};
        MYSQL_RES* pRes;    // 结果集
        sprintf(string, "select * from TalkRoom.talkroom where name='%s';", ev->nowname);
        mysql_real_query(pConn, string, (unsigned int)strlen(string));
        pRes = mysql_store_result(pConn);
        if(pRes){
            if((int)mysql_num_rows(pRes) == 1){//找到一行
                MYSQL_ROW result_row = mysql_fetch_row(pRes);               
                int otherfd = atoi(result_row[4]);
                
                /* 一方结束私聊 */
                if(strcmp(content->valuestring, "bye")==0){
                    Write(ev->fd, ev->buf, strlen(ev->buf));//写给自己
                }
                /*写给对方客户端*/
                cJSON *otherpacket = cJSON_CreateObject();
                cJSON_AddStringToObject(otherpacket, "cmd", "-private_chat-");
                cJSON_AddStringToObject(otherpacket, "othername", ev->nowname);
                cJSON_AddStringToObject(otherpacket, "content", content->valuestring);
                str = cJSON_Print(otherpacket);
                Write(otherfd, str, strlen(str));
                cJSON_Delete(otherpacket);
            }
        }
    }

    printf("client[%d] After dataprocess:\n%s\n", ev->fd, str);
    cJSON_Delete(packet);
}

/* 处理登录 */
char *signIN(struct myevent_s *ev)
{
    /* 解析字符串 */
    cJSON *buf_json = cJSON_Parse(ev->buf);
	if(buf_json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		cJSON_Delete(buf_json);
		return NULL;
	}
    const cJSON *cmd = cJSON_GetObjectItemCaseSensitive(buf_json, "cmd");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(buf_json, "name");
    const cJSON *password = cJSON_GetObjectItemCaseSensitive(buf_json, "password");

    /* 在数据库中查询 */
    char string[1024] = {0};
    MYSQL_RES* pRes;    // 结果集
    sprintf(string, "select * from TalkRoom.talkroom where name='%s' and password='%s';", name->valuestring, password->valuestring);
    int ret = mysql_real_query(pConn, string, (unsigned int)strlen(string));
    if(ret){
        printf("select error:%s\n", mysql_error(pConn));
        return "-mysql error-";
    }
    else{
        pRes = mysql_store_result(pConn);
        if(pRes){
            if((int)mysql_num_rows(pRes) == 1){//找到一行
                /*判断是否已经登录*/
                MYSQL_ROW result_row = mysql_fetch_row(pRes);
                if(atoi(result_row[3]) == 1){
                    return "-Repeat login-";
                }

                /* 登录验证通过,把本次登录的fd加入数据库*/
                bzero(string, sizeof(string));
                sprintf(string, "UPDATE `TalkRoom`.`talkroom` SET `fd`='%d', `online`='1' WHERE `name`='%s';", ev->fd, name->valuestring);              
                mysql_real_query(pConn, string, (unsigned int)strlen(string));
                ev->nowname = name->valuestring;
                return "-verified-";
            }
            else{
                return "-wrong name or password-";
            }
        }
    }
}

/* 处理注册 */
char *signUP(struct myevent_s *ev)
{
    /* 解析字符串 */
    cJSON *buf_json = cJSON_Parse(ev->buf);
	if(buf_json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		cJSON_Delete(buf_json);
		return NULL;
	}
    const cJSON *cmd = cJSON_GetObjectItemCaseSensitive(buf_json, "cmd");
    const cJSON *name = cJSON_GetObjectItemCaseSensitive(buf_json, "name");
    const cJSON *password = cJSON_GetObjectItemCaseSensitive(buf_json, "password");

    /* 在数据库中查询 */
    char string[1024] = {0};
    MYSQL_RES* pRes;    // 结果集
    sprintf(string, "select * from TalkRoom.talkroom where name='%s';", name->valuestring);
    int ret = mysql_real_query(pConn, string, (unsigned int)strlen(string));
    if(ret){
        printf("select error:%s\n", mysql_error(pConn));
        return "-mysql error-";
    }
    else{
        pRes = mysql_store_result(pConn);
        if(pRes){
            if((int)mysql_num_rows(pRes) == 1){//找到一行
                return "-name existed-";
            }
            else{
                /* 注册验证通过 */
                bzero(string, sizeof(string));
                sprintf(string, "INSERT INTO `TalkRoom`.`talkroom` (`fd`, `name`, `password`, `online`, `privatetalk_otherfd`) VALUES ('%d', '%s', '%s', '%d', '%d');", 
                    ev->fd, name->valuestring, password->valuestring, 1, -1);
                mysql_real_query(pConn, string, (unsigned int)strlen(string));
                ev->nowname = name->valuestring;
                return "-verified-";
            }
        }
    }
}

/* 处理私聊 */
char *private_chat(int fd, const char *buf)
{
    /* 解析字符串 */
    cJSON *buf_json = cJSON_Parse(buf);
	if(buf_json == NULL){
		const char *error_ptr = cJSON_GetErrorPtr();
		if(error_ptr != NULL){
			fprintf(stderr, "Error before: %s\n", error_ptr);
		}
		cJSON_Delete(buf_json);
		return NULL;
	}
    const cJSON *selfname = cJSON_GetObjectItemCaseSensitive(buf_json, "selfname");
    const cJSON *othername = cJSON_GetObjectItemCaseSensitive(buf_json, "othername");

    /* 在数据库中查询 */
    char string[1024] = {0};
    MYSQL_RES* pRes;    // 结果集
    sprintf(string, "select * from TalkRoom.talkroom where name='%s' and online='%d';", othername->valuestring, 1);
    mysql_real_query(pConn, string, (unsigned int)strlen(string));
    pRes = mysql_store_result(pConn);
    if(pRes){
        if((int)mysql_num_rows(pRes) == 1){//找到一行
            MYSQL_ROW result_row = mysql_fetch_row(pRes);
            int otherfd = atoi(result_row[0]);
            printf("otherfd=%d\n", otherfd);
            sprintf(string, "UPDATE `TalkRoom`.`talkroom` SET `privatetalk_otherfd`='%d' WHERE `name`='%s';", fd, othername->valuestring);
            mysql_real_query(pConn, string, (unsigned int)strlen(string));
            sprintf(string, "UPDATE `TalkRoom`.`talkroom` SET `privatetalk_otherfd`='%d' WHERE `name`='%s';", otherfd, selfname->valuestring);
            mysql_real_query(pConn, string, (unsigned int)strlen(string));
            return "-verified-";
        }
        else{
            printf("-no name or offline-\n");
            return "-no name or offline-";
        }
    }

}