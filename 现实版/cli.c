// mycli.c

#include "functions.h"

#define PORT 6027    // 端口号
#define MAXSIZE 1024 // 缓冲区大小



int main(int argc, char *argv[])
{
    int actcountact = 0;
    int new_fd;
    char buf[MAXSIZE];
    char rebuf[MAXSIZE];
    char c;
    struct sockaddr_in server_addr; // 定义服务器端套接口数据结构server_addr
    struct hostent *host;           // 定义一个hostent结构的指针
    struct myuser user, me;
    host = gethostbyname("localhost");
    if ((new_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        fprintf(stderr, "Socket Error:%s\a\n", strerror(errno));
        exit(1);
    }
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    if (connect(new_fd, (struct sockaddr *)(&server_addr), sizeof(server_addr)) == -1)
    {
        fprintf(stderr, "Connect Error:%s\a\n", strerror(errno));
        exit(1);
    }
    else
    {
        fprintf(stdout, "连接成功\n");
    }
    while (1)
    {
        printf("请输入用户id：");
        scanf("%d", &me.id);
        printf("请输入用户密码：");
        scanf("%s", me.password);
        write(new_fd, &me, sizeof(me));
        read(new_fd, rebuf, 3);
        if (strcmp(rebuf, "ER") == 0)
        {
            printf("密码错误！\n");
        }
        else if (strcmp(rebuf, "NO") == 0)
        {
            printf("账号不存在！\n");
        }
        else if (strcmp(rebuf, "L1") == 0)
        {
            me.status = 1;
            break;
        }
        else if (strcmp(rebuf, "L2") == 0)
        {
            me.status = 0;
            break;
        }
    }
    read(new_fd, me.name, sizeof(me.name));
    if (me.status)
    {
        printf("管理员%s您好，欢迎您使用报账系统!\n", me.name);
    }
    else
    {
        printf("用户%s您好，欢迎您使用报账系统!\n", me.name);
    }
    while (1)
    {
        printf("     *活动报账管理系统*\n");
        printf("-------------------------\n");
        printf("     1.进入用户管理       \n");
        printf("     2.进入报账系统       \n");
        printf("     0.退出系统           \n");
        printf("-------------------------\n");
        printf("请输入：");
        scanf("%s", buf);
        write(new_fd, buf, sizeof(buf)); // 发送消息
        if (buf[0] == '1')
        {
            while (1)
            {
                printf("       *用户管理平台*       \n");
                printf("---------------------------\n");
                printf("     1.添加新用户账号       \n");
                printf("     2.查找用户是否存在     \n");
                printf("     3.查看全部账号         \n");
                printf("     4.删除用户账号         \n");
                printf("     5.修改用户账号         \n");
                printf("     0.返回                \n");
                printf("---------------------------\n");
                printf("请输入：");
                scanf("%s", buf);
                if (buf[0] == '1' && me.status != 1)
                {
                    printf("对不起，您的权限不允许您添加用户\n");
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                    continue;
                }
                else if (buf[0] == '4' && me.status != 1)
                {
                    printf("对不起，您的权限不允许您删除用户\n");
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                    continue;
                }
                else if (buf[0] == '5' && me.status != 1)
                {
                    printf("对不起，您的权限不允许您修改用户\n");
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                    continue;
                }
                write(new_fd, buf, sizeof(buf)); // 发送消息
                if (buf[0] == '0')
                {
                    break;
                }
                else if (buf[0] == '1')
                {
                    int id;
                    read(new_fd, rebuf, sizeof(rebuf));                // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    scanf("%s", buf);
                    write(new_fd, buf, sizeof(buf));      // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    if (strcmp(rebuf, "用户已存在!") == 0)
                    {
                        printf("输入任意键继续...\n");
                        scanf("%s", buf);
                        continue;
                    }
                    scanf("%d", &id);
                    write(new_fd, (char*)&id, sizeof(id));      // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    scanf("%s", buf);
                    write(new_fd, buf, sizeof(buf));                 // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));              // 接收新消息
                    printf("收到服务器消息:%s\n请输入y/n：", rebuf); // 输出到终端
                    scanf("%s", buf);
                    write(new_fd, buf, sizeof(buf));      // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if (buf[0] == '2')
                {
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    scanf("%s", rebuf);
                    write(new_fd,rebuf , sizeof(rebuf));      // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if (buf[0] == '3')
                {
                    printf("----------------用 户 列 表----------------\n");
                    read(new_fd, rebuf, sizeof(struct myuser)); // 接收新消息
                    while (strcmp(rebuf, "end") != 0)
                    {
                        struct myuser *ur = (struct myuser *)&rebuf;
                        if (ur->status)
                        {
                            printf("用户名：%s  身份权限：管理员\n", ur->name);
                        }
                        else
                        {
                            printf("用户名：%s  身份权限：用户\n", ur->name);
                        }
                        write(new_fd, "OK", 3);
                        read(new_fd, rebuf, sizeof(struct myuser));
                    }
                    printf("------------------到 底 啦------------------\n");
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if (buf[0] == '4')
                {
                    int id;
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    scanf("%d", &id);
                    write(new_fd, (char*)&id, sizeof(id));               // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if (buf[0] == '5')
                {
                    read(new_fd, rebuf, sizeof(rebuf));
                    printf("收到服务器消息:%s\n", rebuf);
                    int id;
                    scanf("%d", &id);
                    write(new_fd, (char*)&id, sizeof(id));

                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    scanf("%s", buf);
                    write(new_fd, buf, sizeof(buf));      // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    scanf("%s", buf);
                    write(new_fd, buf, sizeof(buf));      // 发送消息
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
            }
        }

        else if (buf[0] == '2')
        {
            while (1)
            {
                printf("      *报账管理平台*        \n");
                printf("---------------------------\n");
                printf("     1.查询活动     \n");
                printf("     2.添加活动           \n");
                printf("     3.删除活动           \n");
                printf("     4.修改活动            \n");
                printf("     0.返回                \n");
                printf("---------------------------\n");
                printf("请输入：");
                scanf("%s", buf);
                if (buf[0] == '3' && me.status != 1)
                {
                    printf("对不起，您的权限不允许您添加活动\n");
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                    continue;
                }
                else if (buf[0] == '4' && me.status != 1)
                {
                    printf("对不起，您的权限不允许您删除活动\n");
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                    continue;
                }
                
                write(new_fd, buf, sizeof(buf)); // 发送消息
                if (buf[0] == '1')
                {
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                        printf("----------------账 目 列 表----------------\n");
                        read(new_fd, rebuf, sizeof(struct act)); // 接收新消息
                        while (strcmp(rebuf, "end") != 0)
                        {
                            struct act *tmp = (struct act *)&rebuf;
                            printf("活动名称:%s\n",tmp->name);
                            printf("活动地点:%s\n",tmp->location);
                            printf("活动开始时间:%ld\n",tmp->btime);
                            printf("活动结束时间:%ld\n",tmp->etime);
                            printf("活动备注：%s\n",tmp->comment);
                            if(tmp->auditing==0)
                            {printf("活动审核情况:审核中\n");}
                            if(tmp->auditing==1) {
                                printf("活动审核情况:审核通过\n");
                            }
                            if(tmp->auditing==-1) {
                                printf("活动审核情况:驳回\n");
                                }
                            printf("审核人id:%d\n",tmp->auditior);
                            printf("签到码:%d\n",tmp->checkin);
                            printf("\n");

                            printf("******************************************************************************************\n");
                            write(new_fd, "OK", 3);
                            read(new_fd, rebuf, sizeof(struct act));
                        }
                            
                        printf("------------------到 底 啦------------------\n");
                    
                }
                else if(buf[0]=='2')
                {
                    struct act tmp_b;
                    
                    printf("请输入活动名称：");
                    scanf("%s", tmp_b.name);
                    send(new_fd, tmp_b.name, sizeof(tmp_b.name), 0);
                    printf("请输入活动地点：");
                    scanf("%s", tmp_b.location);
                    send(new_fd, &tmp_b.location, sizeof(tmp_b.location), 0);
                    printf("hhh%s\n", tmp_b.location);
                    printf("请输入活动开始时间：");
                    scanf("%ld", &tmp_b.btime);
                    send(new_fd, (char*)&tmp_b.btime, sizeof(tmp_b.btime), 0);
                    printf("请输入活动结束时间：");
                    scanf("%ld", &tmp_b.etime);
                    send(new_fd, (char*)&tmp_b.etime, sizeof(tmp_b.etime), 0);
                    printf("请输入备注：");
                    scanf("%s", tmp_b.comment);
                    send(new_fd, tmp_b.comment, sizeof(tmp_b.comment), 0);
                    tmp_b.auditing = 0;
                    send(new_fd, (char*)&tmp_b.auditing, sizeof(tmp_b.auditing), 0);
                    tmp_b.applicant = me.id;
                    send(new_fd, (char*)&tmp_b.applicant, sizeof(tmp_b.applicant), 0);
                    tmp_b.auditior = -1;
                    send(new_fd, (char*)&tmp_b.auditior, sizeof(tmp_b.auditior), 0);
                    tmp_b.id = ++actcountact;
                    send(new_fd, (char*)&tmp_b.id, sizeof(tmp_b.id), 0);
                    tmp_b.checkin = -1;
                    send(new_fd, (char*)&tmp_b.checkin, sizeof(tmp_b.checkin), 0);

                
                    printf("xxx%s\n", tmp_b.location);

                    read(new_fd, rebuf, MAXSIZE);         // 接收新消息
                    
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if(buf[0]=='3')
                {
                    read(new_fd, rebuf, sizeof(rebuf)); // 接收新消息
                    printf("%s", rebuf);
                    int id; // 输出到终端
                    scanf("%d", &id);
                    write(new_fd, (char*)&id, MAXSIZE);
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if(buf[0]=='4')
                {
                    read(new_fd, rebuf, sizeof(rebuf)); // 接收新消息
                    printf("%s", rebuf);
                    int id; // 输出到终端
                    scanf("%d", &id);
                    write(new_fd, (char*)&id, MAXSIZE);
                    read(new_fd, rebuf, sizeof(rebuf));   // 接收新消息
                    printf("收到服务器消息:%s\n", rebuf); // 输出到终端
                    printf("输入任意键继续...\n");
                    scanf("%s", buf);
                }
                else if (buf[0] == '0')
                {
                    break;
                }
                else
                {
                    printf("请输入正确的命令...\n");
                }
            }
        }

    }
    printf("结束通信\n");
    close(new_fd);
    return 0;
}
