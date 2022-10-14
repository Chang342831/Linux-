#include <sys/types.h>
#include <sys/socket.h>							// 包含套接字函数库
#include <stdio.h>
#include <netinet/in.h>							// 包含AF_INET相关结构
#include <arpa/inet.h>							// 包含AF_INET相关操作的函数
#include <unistd.h>
#include <pthread.h>
#include "functions.h"
#define PORT 3339
int main()
{
    int type = -1;
    void *msg = NULL;
    int s_fd, c_fd;	// 服务器和客户套接字标识符
    int s_len, c_len;			// 服务器和客户消息长度
    struct sockaddr_in s_addr;	// 服务器套接字地址
    struct sockaddr_in c_addr;	// 客户套接字地址
    s_fd = socket(AF_INET, SOCK_STREAM, 0); // 创建套接字
    s_addr.sin_family = AF_INET;	// 定义服务器套接字地址中的地址域为IPv4
    s_addr.sin_addr.s_addr=htonl(INADDR_ANY); // 定义套接字地址
    s_addr.sin_port = htons(PORT); // 定义服务器套接字端口
    s_len = sizeof(s_addr);
    if (bind(s_fd, (struct sockaddr *) &s_addr, s_len) < 0) {
        printf("bind error...\n");
        exit(1);
    }
    printf("bind port successfully.\n");
    printf("Local port: %d\n", PORT);

    listen(s_fd, 5);   // 设置套接口监听状态
    printf("Listening...\n");

    while(1) {   // 只能同时处理一个请求
        c_fd = accept(s_fd, (struct sockaddr *)(&c_addr), (socklen_t *__restrict)(&c_len));
        // 处理登录
        user thisuser;
        int response_login;
       

        // 多线程服务器实现






        while(1) {
            if(read(c_fd, &type, sizeof(type)) > 0) {   // 接收请求类型
                switch (type) {
                    case 0: {   // 注册
                        sign_msg msg_log;
                        int response;
                        while(1) {
                            if(read(c_fd, &msg_log, sizeof(msg_log)) > 0) {
                                break;
                            }
                        }
                        response = adduser();   // 调用functios.h里的函数
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                    case 1: {   // 登录
                        sign_msg msg_log;
                        int response;
                        while(1) {
                            if(read(c_fd, &msg_log, sizeof(msg_log)) > 0) {
                                break;
                            }
                        }
                        response = login_decision(&thisuser, );   // 调用functios.h里的函数
                        write(c_fd, &thisuser, sizeof(thisuser));
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                    case 2: {   // 查询用户所申请的活动
                        act temp[20];
                        int response;
                        user my_user;
                        while(1) {
                            if(read(c_fd, &my_user, sizeof(my_user)) > 0) {
                                break;
                            }
                        }
                        int count = searchbyuser(AFILE, myuser.id, temp);   // 调用functios.h里的函数
                        if (count == 0) {   // 未查询到
                            response = -1;
                            write(c_fd, &temp, sizeof(temp));
                            write(c_fd, &response, sizeof(response));
                            break;
                        }
                        else {
                            response = count;
                            write(c_fd, &temp, sizeof(temp));
                            write(c_fd, &response, sizeof(response));
                        }
                        
                        break;
                    }
                    case 3: {   // 查询指定时间段的活动
                        search_msg s_msg;
                        act temp[20];
                        int response;
                        while(1) {
                            if(read(c_fd, &s_msg, sizeof(s_msg)) > 0) {
                                break;
                            }
                        }
                        int count = bill_query(AFILE, s_msg.btime, s_msg.etime, temp);   // 调用functios.h里的函数
                        if (count == 0) {
                            response = -1;
                            write(c_fd, &temp, sizeof(temp));
                            write(c_fd, &response, sizeof(response));
                        }
                        else {
                            response = count;
                            write(c_fd, &temp, sizeof(temp));
                            write(c_fd, &response, sizeof(response));
                        }
                        break;
                    }
                    case 4: {   // 申请活动
                        act temp;
                        int response;
                        while(1) {
                            if(read(c_fd, &temp, sizeof(temp)) > 0) {
                                break;
                            }
                        }
                        response = bill_insert(AFILE, &temp, thisuser.status);   // 调用functios.h里的函数
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                    case 5: {   // 删除活动
                        int actid;
                        int response;
                        while(1) {
                            if(read(c_fd, &actid, sizeof(actid)) > 0) {
                                break;
                            }
                        }
                        response = bill_remove(AFILE, actid, thisuser.status, thisuser.id);   // 调用functios.h里的函数
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                    case 6: {   // 删除账户
                        int deleteid;
                        int response;
                        while(1) {
                            if(read(c_fd, &deleteid, sizeof(deleteid)) > 0) {
                                break;
                            }
                        }
                        response = deleteuser(deleteid, thisuser.status);   // 调用functios.h里的函数
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                    case 7: {   // 查询账户
                        user temp;
                        int checkid;
                        int response;
                        while(1) {
                            if(read(c_fd, &checkid, sizeof(checkid)) > 0) {
                                break;
                            }
                        }
                        response = searchuser(checkid);   // 调用functios.h里的函数
                        if (response == 1) {
                            temp = searchuser_item(checkid);   // 调用functios.h里的函数（新增）
                        }
                        write(c_fd, &temp, sizeof(temp));
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                    case 8: {   // 审核活动
                        int response;
                        int count = 0;
                        act actlist[20];
                        count = actauditing1(AFILE, actlist);
                        if (count == 0) {
                            response = -1;
                            write(c_fd, actlist, sizeof(actlist));
                            write(c_fd, &response, sizeof(response));
                        }
                        else {
                            int id;
                            int idea;
                            response = 1;
                            write(c_fd, actlist, sizeof(actlist));
                            write(c_fd, &response, sizeof(response));
                            write(c_fd, &id, sizeof(id));
                            write(c_fd, &idea, sizeof(idea));
                            actauditing1(AFILE, id, idea);
                        }
                        break;
                    }
                    case 9: {   // 更改用户权限
                        int id;
                        int response;
                        while(1) {
                            if(read(c_fd, &id, sizeof(id)) > 0) {
                                break;
                            }
                        }
                        response = updateuser(id);
                        write(c_fd, &response, sizeof(response));
                        break;
                    }
                }

            }
        }
        close(c_fd);   //关闭当前请求的套接字
        printf("End Connection.\n");
        printf("Listening...\n");
    }
    
    return 0;
}