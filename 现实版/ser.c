// myser.c

#include "function.h"

#define PORT 7812	 // 端口号
#define MAXSIZE 10240 // 缓冲区大小

int running = 1;

// void stop()
// {
// 	running = 0;
// }

int main(int argc, char *argv[])
{
	int sockfd, new_fd;				// 定义存放套接口描述符的变量
	struct sockaddr_in server_addr; // 定义服务器端套接口数据结构server_addr
	struct sockaddr_in client_addr; // 定义客户端套接口数据结构client_addr
	struct myuser user;
	struct myuser ur;
	int server_len, client_len; // 服务器和客户消息长度
	int nbytes, portnumber;
	int num;
	int flagt = 0;
	char name[10];
	char buf[MAXSIZE];
	char rebuf[MAXSIZE];
	char pathname[100];
	// signal(SIGINT, stop); // 注册SIGINT信号
	FILE *fpn, *fp;
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) // 服务器端开始建立socket描述符
	{
		fprintf(stderr, "Socket error:%s\n\a", strerror(errno));
		exit(1);
	}
	// bzero(&server_addr, sizeof(struct sockaddr_in));									// 先将套接口地址数据结构清零
	server_addr.sin_family = AF_INET;													// 设为TCP/IP地址族
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);									// 设置本机地址并从主机字节序转换为网络字节序*/
	server_addr.sin_port = htons(PORT);													// 设置端口号并从主机字节序转换为网络字节序*/
	if (bind(sockfd, (struct sockaddr *)(&server_addr), sizeof(struct sockaddr)) == -1) // 调用bind函数绑定指定的端口号和ip地址到服务器创建的套接口
	{
		fprintf(stderr, "Bind error:%s\n\a", strerror(errno));
		exit(1);
	}
	if (listen(sockfd, 5) == -1) // 端口绑定成功，监听sockfd描述符，设置同时处理的最大连接请求数为5 */
	{
		fprintf(stderr, "Listen error:%s\n\a", strerror(errno));
		exit(1);
	}
	while (running)
	{
		client_len = sizeof(client_addr); // 接收客户端连接请求
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, (socklen_t *__restrict)&client_len);
		while (1)
		{
			read(new_fd, rebuf, sizeof(struct myuser));
			// printf("%s\n", rebuf);
			memcpy(&ur, rebuf, sizeof(ur));
			printf("%d,%s\n", ur.id, ur.password);
			struct myuser user;
			FILE *fp;
			int found = 0;
			if ((fp = fopen(ARFILE, "rb")) == NULL)
			{
				return 0;
			}
			while (fread(&user, sizeof(user), 1, fp) == 1)
			{
				if (ur.id==user.id)
				{
					found = 1;
					break;
				}
				memset(&user, 0x00, sizeof(user));
			}
			fclose(fp);
			if (found)
			{
				if (strcmp(ur.password, user.password) == 0)
				{
					if (user.status==1)
					{
						write(new_fd, "L1", 3);
					}
					else
					{
						write(new_fd, "L2", 3);
					}
					struct myuser tmp3;
					tmp3 = searchus(ARFILE, ur.id);
					write(new_fd, tmp3.name, sizeof(tmp3.name));
					break;
				}
				else
				{
					write(new_fd, "ER", 3);
				}
			}
			else
			{
				write(new_fd, "NO", 3);
			}
			
		}
		while (1)
		{
			memset(rebuf, 0x00, sizeof(rebuf));
			read(new_fd, rebuf, sizeof(rebuf));
			printf("收到客户端消息: %s\n", rebuf); // 输出到终端
			if (rebuf[0] == '1')
			{
				while (1)
				{
					memset(rebuf, 0x00, sizeof(rebuf));
					read(new_fd, rebuf, sizeof(rebuf));
					printf("收到客户端消息: %s\n", rebuf); // 输出到终端
					if (rebuf[0] == '1')
					{
						int id;
						write(new_fd, "请输入新用户名", MAXSIZE); // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						strcpy(user.name, rebuf);
						
						printf("收到客户端用户名消息: %s\n", user.name); // 输出到终端
						if (queryuser(user.name) == 1)
						{
							write(new_fd, "用户已存在!", MAXSIZE);
							continue;
						}
						
						write(new_fd, "请输入新用户ID", MAXSIZE); // 回复消息
						read(new_fd, (char*)&id, sizeof(id));
						user.id = id;

						write(new_fd, "请输入用户密码", MAXSIZE); // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						strcpy(user.password, rebuf);
						printf("收到客户端密码信息: %s\n", user.password);					  // 输出到终端
						write(new_fd, "如果该新添用户是管理员请输入y，否则输入n：", MAXSIZE); // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						printf("收到客户端权限消息: %s\n", rebuf); // 输出到终端
						if (rebuf[0] == 'y')
						{
							user.status = 1;
						}
						else if (rebuf[0] == 'n')
						{
							user.status = 0;
						}
						if (insertuser(user))
						{
							write(new_fd, "增加新用户成功...\n", MAXSIZE);
						}
						else
						{
							write(new_fd, "添加用户的时候发生了错误...\n", MAXSIZE);
						}
					}
					else if (rebuf[0] == '2')
					{
                        
						write(new_fd, "请输入用户名", MAXSIZE); // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						printf("收到客户端查询用户是否存在消息: %s\n", rebuf); // 输出到终端
						if (queryuser(rebuf))
						{
							write(new_fd, "该用户存在\n", MAXSIZE);
						}
						else
						{
							write(new_fd, "没有找到该用户的数据...\n", MAXSIZE);
						}
					}
					else if (rebuf[0] == '3')
					{
						memset(&user, 0x00, sizeof(user));
						FILE *fp;
						if ((fp = fopen(ARFILE, "rb")) == NULL)
						{
							return 0;
						}
						while (fread(&user, sizeof(user), 1, fp) == 1)
						{
							write(new_fd, &user, sizeof(user));
							read(new_fd, rebuf, sizeof(rebuf));
						}
						fclose(fp);
						write(new_fd, "end", 4);
					}
					else if (rebuf[0] == '4')
					{
						write(new_fd, "请输入要删除的用户姓名:", MAXSIZE); // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						printf("收到客户端删除用户姓名消息: %s\n", rebuf); // 输出到终端
						int ret = removeuser(rebuf);
						if (ret == 1)
						{
							write(new_fd, "删除用户账号成功...\n", MAXSIZE);
						}
						else if (ret == -1)
						{
							write(new_fd, "不能删除管理员账号...\n", MAXSIZE);
						}
						else
						{
							write(new_fd, "删除用户账号发生了错误...\n", MAXSIZE);
						}
					}
					else if (rebuf[0] == '5')
					{
						int id;
						write(new_fd, "请输入用户ID", MAXSIZE); // 回复消息
						read(new_fd, (char*)&id, sizeof(id));
						user.id = id;
						write(new_fd, "请输入用户姓名", MAXSIZE); // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						strcpy(user.name, rebuf);
						printf("收到客户端修改用户名消息: %s\n", rebuf); // 输出到终端
						write(new_fd, "请输入用户密码", MAXSIZE);		 // 回复消息
						read(new_fd, rebuf, sizeof(rebuf));
						strcpy(user.password, rebuf);
						printf("收到客户端修改密码消息: %s\n", rebuf); // 输出到终端
						int ret = updateuser(user);
						if (ret == 1)
						{
							write(new_fd, "修改用户资料成功...\n", MAXSIZE);
						}
						else if (ret == -1)
						{
							write(new_fd, "没有该用户的数据...\n", MAXSIZE);
						}
						else if (ret == -2)
						{
							write(new_fd, "没有权限修改管理员账号...\n", MAXSIZE);
						}
						else
						{
							write(new_fd, "修改用户账号发生了错误...\n", MAXSIZE);
						}
					}
					else if (rebuf[0] == '0')
					{
						break;
					}
				}
			}

			else if (rebuf[0] == '2')
			{
				while (1)
				{
					memset(rebuf, 0x00, sizeof(rebuf));
					while(1) {
						if (recv(new_fd, rebuf, sizeof(rebuf), 0) > 0) {
							break;
						}
					}
					printf("%s\n", rebuf);
					if (rebuf[0] == '1')
					{
						write(new_fd, "加载成功", MAXSIZE);
						struct act myact;
						memset(&myact, 0x00, sizeof(myact));
						strcpy(pathname, BRFILE);
						FILE *fpl;
						if ((fpl = fopen(BRFILE, "rb")) == NULL) {
							return 0;
						}
						while(fread(&myact, sizeof(myact), 1, fpl) == 1) {
							write(new_fd, &myact, sizeof(myact));
							read(new_fd, rebuf, sizeof(rebuf));
						}
						fclose(fpl);
                        
						write(new_fd, "end", 4);
						
					}
					else if(rebuf[0]=='2')
					{
						printf("懂你意思！\n");
						struct act tmp_b;
						
						recv(new_fd, tmp_b.name, sizeof(tmp_b.name), 0);
						printf("%s\n", tmp_b.name);
						recv(new_fd, tmp_b.location, sizeof(tmp_b.location), 0);
						printf("%s\n", tmp_b.location);
						recv(new_fd, (char*)&tmp_b.btime, sizeof(tmp_b.btime), 0);
						recv(new_fd, (char*)&tmp_b.etime, sizeof(tmp_b.etime), 0);
						recv(new_fd, tmp_b.comment, sizeof(tmp_b.comment), 0);
						printf("%s\n", tmp_b.comment);	
						recv(new_fd, (char*)&tmp_b.auditing, sizeof(tmp_b.auditing), 0);
						recv(new_fd, (char*)&tmp_b.applicant, sizeof(tmp_b.applicant), 0);
						recv(new_fd, (char*)&tmp_b.auditior, sizeof(tmp_b.auditior), 0);
						recv(new_fd, (char*)&tmp_b.id, sizeof(tmp_b.id), 0);
						recv(new_fd, (char*)&tmp_b.checkin, sizeof(tmp_b.checkin), 0);
					
						actinsert(tmp_b);
						write(new_fd, "添加成功!", MAXSIZE);

					}
					else if(rebuf[0]=='3')
					{
						write(new_fd, "请输入要删除的活动:", MAXSIZE);
						int id;
						read(new_fd, (char*)&id, MAXSIZE);
						actremove(id);
						
						close(new_fd);
						flagt = 1;
						break;
					}
					else if(rebuf[0]=='4')
					{
						write(new_fd, "请输入要审核的活动：", MAXSIZE);
						int id;
						read(new_fd, (char*)&id, MAXSIZE);
						int i = search( id);
						if (i == -1)
						{
							write(new_fd, "该活动不存在!", MAXSIZE);
						}
						else
						{
							if (actupdate(ur.id, i))
							{
								write(new_fd, "审核成功!", MAXSIZE);
							}
							else
							{
								write(new_fd, "审核失败，该活动已审核!", MAXSIZE);
							}
						}
					}
					else
					{
						break;
					}
				}
			}
			else /*if (rebuf[0] == '0')*/
			{
				break;
			}
		}
		if (flagt) {
			close(new_fd);
		}
	}
	fp = fopen("./cata.txt", "w+b"); // 写入活动信息
	int i = 0;
	
	fclose(fp);
	close(sockfd);
	return 0;
}