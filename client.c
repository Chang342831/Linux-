#include <sys/types.h>
#include <sys/socket.h>	 // 包含套接字函数库
#include <stdio.h>
#include <netinet/in.h>	 // 包含AF_INET相关结构
#include <arpa/inet.h>	 // 包含AF_INET相关操作的函数
#include <unistd.h>
#include "functionsv4.h"
#define PORT 5566

int mysend_recv(int sockfd, int type, void *msg);
void initPage();
void mainPage(int sockfd, user myuser);
user sign_up();
sign_msg* sign_in();
void search_activity(int sockfd, user myuser, int type);
void engine_activity(int sockfd, user myuser);
void delete_activity(int sockfd, user myuser);
void check_profile(user myuser);
void delete_user(int sockfd, user myuser);
void check_user(int sockfd, user myuser);
void audit_activity(int sockfd, user myuser);
void update_permission(int sockfd, user myuser);

int main() {
   // 定义网络通信变量
   int sockfd;	 // 客户套接字标识符
   int len;	 // 客户消息长度
   struct sockaddr_in addr;	 // 客户套接字地址
   int newsockfd;
   char buf[256]="come on!";//要发送的消息
   int len2;							
   char rebuf[256];  
   sockfd = socket(AF_INET,SOCK_STREAM, 0);	// 创建套接字
   addr.sin_family = AF_INET; // 客户端套接字地址中的域
   addr.sin_addr.s_addr=htonl(INADDR_ANY);   
   addr.sin_port = htons(PORT); // 客户端套接字端口
   len = sizeof(addr);

   // 寻求与服务器建立连接
   newsockfd = connect(sockfd, (struct sockaddr *) &addr, len);	// 发送连接服务器的请求
   if (newsockfd == -1) {
      perror("连接失败");
      return 1;
   }
   // 成功与服务器建立连接
   printf("与服务器成功建立连接\n");
   sleep(0.5);
   for (int i = 0; i < 3; i++) {   // 打印三个点，停顿一下
      printf(". ");
      sleep(0.5);
   }
   initPage();

   // 变量定义
   int response = 100;

   // 用户登陆界面
   while(1) {
      sign_msg *msg_log;
      user my_user;   // 初始化用户权限
      int start_type = 0;
      printf("(1) Press 1 to Sign in.    (2)Press 2 to Sign up    (3)Press 0 to Quit\n");
      scanf("%d", &start_type);
      
      if (start_type == 1) {   // 进入登录界面
         while(1) {
            int type = 1;
            msg_log = sign_in();   // 获取登录信息
            write(sockfd, &type, sizeof(type));   
            write(sockfd, &msg_log, sizeof(msg_log));   // 向服务器发出1号请求（登录），并接收服务器的响应值. msg = sign_msg结构体
            while (1)
            {
               if (read(sockfd, &my_user, sizeof(my_user)) > 0) {  // 若登录成功，服务器会额外返回一个user结构体，用于后续函数的调用
                  break;
               }
            }
            while (1)
            {
               if (read(sockfd, &response, sizeof(response)) > 0) {   // 等待响应
                  break;
               }
            }

            if (response == -1) {   // 返回值为-1，说明账号未注册
               printf("Unregistered account.  Please sign up first.\n\n");
               break;
            }
            else if (response == -2) {   // 返回值为-2，说明账号已注册，但是密码错误
               printf("Wrong password.  Please try again.\n");
               continue;
            }
            else {   // 返回值为1，说明账号密码均正确，将读取自身的用户结构，同时进入下一界面
               mainPage(sockfd, my_user);   // mainPage内部实现了循环，这里只嵌套了两层
               break;
            }
         }
      }
      else if (start_type == 2){   // 进入注册界面
         int type = 0;
         int response;
         user theuser;
         initUser(theuser);
         theuser = sign_up();
         write(sockfd, &type, sizeof(type));   
         write(sockfd, &theuser, sizeof(theuser));   // 向服务器发出0号请求（注册），并接收服务器的响应值
         while(1) {
               if(read(sockfd, &response, sizeof(response)) > 0) {
                  break;
               }
         }
         if (response == 0) {
            printf("Duplication of name. Please change another account. Please Try again\n\n");
         }
         else {
            printf("Successfully signed up. Now Login to your account......\n");
            sleep(0.5);
         }
      }
      else {   // 退出程序
         break;
      }
   }
   close(sockfd);   // 关闭连接

   return 0;
}

int mysend_recv(int sockfd, int type, void *msg) {
   int typet[1] = {0};
   int response[1] = {0};
   typet[0] = type;   // 初始化消息类型type
   
   write(newsockfd, typet, sizeof(typet));
   write(newsockfd, msg, sizeof(msg));

   while (1)
   {
      if (read(newsockfd, response, sizeof(response)) > 0) {   // 等待响应
         break;
      }
   }
   return response[0];   // 返回服务器相应结果
}

void initPage() {
   printf("Fzu Volunteer Online Platform\n");
   printf("Guide\n");
}

void mainPage(int sockfd, user myuser) {
   system("clear");   // 清空屏幕

   char choice;   // 初始化用户操作选择

   int run_flag = 1;
   while(run_flag) {
      printf("Main Page.\n\n");
      printf("Welcome, user: %s\n", user.name);
      printf("Functions: \n");
      printf("     (1)Search Volunteer Activity nearby\n");   // 操作一：查找志愿活动
      printf("     (2)Engine a Volunteer Activity\n");   // 操作二：发起一个志愿活动   （略微区别管理员和普通用户（auditing码不同））
      printf("     (3)Delete Volunteer Activity\n");   // 操作三：删除已发起的志愿活动 （区别管理员和普通用户）
      printf("     (4)Check your Profile\n");   // 操作四：查看自己的档案

      // 以下为管理员特有的操作
      if (myuser.status >= 1) {
         printf("     (5)Delete Account\n");   // 操作五：删除账号 （区别管理员和普通用户（删自己或删他人））
         printf("     (6)Check user\n")   // 操作六：查看指定用户的信息
         printf("     (7)Activity Audit\n")   // 操作七：审核活动
         
         if (myuser.status == 2) {   // 根用户能执行的操作
            printf("     (8)Update Permission\n")   // 操作八：权限更新
         }
      }
      printf("(*)Log out\n");


      printf("\n");
      printf("Please choose your operation(the index number): \n");

      char = getchar();
      switch(choice) {
         case '1': {
            int type;
            printf("\n");
            printf("(1)Search activities recently;     (2)Search activities engined by you     (0)Back\n");
            printf("Please choose: ");
            scanf("%d", &type);
            search_activity(sockfd, myuser, type);   // 调用活动查找函数
            break;
         }
         case '2': {
            engine_activity(sockfd, myuser);   // 调用活动发起函数
            break;
         }
         case '3': {
            delete_activity(sockfd, myuser);   // 调用活动删除函数
            break;
         }
         case '4': {
            check_profile(myuser);   // 调用用户信息查询函数
            break;
         }
         case '5': {
            if (myuser.status < 1) {
               printf("ILLEGAL opeartion. Please try again\n"); // 用户执行非授权操作，为非法操作
            }
            else {
               delete_account(sockfd, myuser);   // 删除账号
            }
            break;
         }
         case '6': {
            if (myuser.status < 1) {
               printf("ILLEGAL opeartion. Please try again\n"); // 用户执行非授权操作，为非法操作
            }
            else {
               check_user(sockfd, myuser);
            }
            break;
         }
         case '7': {
            if (myuser.status < 1) {
               printf("ILLEGAL opeartion. Please try again\n"); // 用户执行非授权操作，为非法操作
            }
            else {
               audit_activity(sockfd, myuser);
            }
            break;
         }
         case '8': {
            if (myuser.status < 2) {
               printf("ILLEGAL opeartion. Please try again\n"); // 用户执行非授权操作，为非法操作
            }
            else {
               update_permission(sockfd, myuser);
            }
            break;
         }
         case '*': {
            run_flag = 0;
            printf("  Thank you\n");
            break;
         }
         default: {
            printf("ILLEGAL opeartion. Please try again\n");
            break;
         }
      }
   }
}

sign_msg* sign_in() {   //登陆交互界面，待完善。
   int account;
   char passwd[10] = "0";
   char ch;
   printf("Please type your account: ");
   scanf("%d", &account);
   printf("Please type your password: ");
   scanf("%s", &passwd);
   ch = getchar();

   sign_msg login;
   initSign_msg(login);
   login.account = account;
   strcpy(login.passwd, passwd);

   return &login;
}

user sign_up(user theuser) {   //登陆交互界面，待完善。
   initUser(theuser);
   while(1) {
      printf("Please type your phone number: ");
      scanf("%s", &theuser.phone);
      ch = getchar();
      printf("Please type your name/account that would be used for login: ");
      scanf("%d", &theuser.name);
      printf("Please type your id that would be used for login: ");
      scanf("%d", &theuser.id);
      printf("Please type your password: ");
      scanf("%s", &temp);
      ch = getchar();
      printf("Please type your password again: ");
      scanf("%s", &theuser.passwd);
      ch = getchar();
      if (strcmp(passwd, temp)  != 0) {
         printf("Two inputs of password doesn't fit. Please try again\n\n");
         continue;
      }
   }

   return theuser;
}

void search_activity(int sockfd, user myuser, int type){
   int typet = -1;
   int response = 0;
   if (type == 2) {
      act tmp[20];
      write(sockfd, &type, sizeof(type));   // 向服务器发送2号请求（查询自己申请的活动），msg = user结构体
      write(sockfd, &myuser, sizeof(myuser));
      while (1)
      {
         if (read(sockfd, &tmp, sizeof(tmp)) > 0) {   // 等待响应
            break;
         }
      }
      while (1)
      {
         if (read(sockfd, &response, sizeof(response)) > 0) {   // 等待响应
            break;
         }
      }
      if (response == -1) {   // 未查询到活动
         printf("You don't engine any activities.\n");
      }
      else {
         for (int i = 0; i < response; i++) {   // response为该用户申请的活动数量
            printf("活动%d:\n",i + 1);
            printf("活动名称:%s\n",tmp[i].name);
            printf("活动地点:%s\n",tmp[i].location);
            printf("活动开始时间:%ld\n",tmp[i].btime);
            printf("活动结束时间:%ld\n",tmp[i].etime);
            printf("活动备注：%s\n",tmp[i].comment);
            if(tmp.auditing==0)
               {printf("活动审核情况:审核中\n");}
            if(tmp.auditing==1) {
               printf("活动审核情况:审核通过\n");
            }
            if(tmp.auditing==-1) {
               printf("活动审核情况:驳回\n");
            }
            printf("审核人id:%d\n",tmp[i].auditior);
            printf("签到码:%d\n",tmp[i].checkin);
            printf("\n");
         }
      }
      // searchbyuser(AFILE, myuser.id);
   }
   else if (type == 1) {
      act tmp2[20];
      search_msg s_msg;
      
      int len2 = 0;
      long btime = 0;
      long etime = 0;
      printf("Time range\n   (Format: 年月日小时分钟, 例如: 2022(年份)09(月份)05(日期)09(小时)30(分钟)其中前导0不可省略)\n");
      printf("begin time: ");
      scanf("%ld", &btime);
      printf("end time: ");
      scanf("%ld", &etime);
      s_msg.btime = btime;
      s_msg.etime = etime;

      write(sockfd, &type, sizeof(type));   // 向服务器发送3号请求（查询指定时间段的活动），msg = search_msg结构体
      write(sockfd, &s_msg, sizeof(s_msg));
      while (1)
      {
         if (read(sockfd, &tmp2, sizeof(tmp2)) > 0) {   // 等待响应
            break;
         }
      } 
      // bill_query(AFILE, btime, etime);
      while (1)
      {
         if (read(sockfd, response, sizeof(response)) > 0) {   // 等待响应
            break;
         }
      }
      if (response == -1) {   // 未查询到活动
         printf("No such activity.\n");
      }
      else {   // 查询到活动，response = 活动的数量
         for (int i = 0; i < response; i++) {
            printf("活动%d:\n",i + 1);
            printf("活动名称:%s\n",tmp2[i].name);
            printf("活动地点:%s\n",tmp2[i].location);
            printf("活动开始时间:%ld\n",tmp2[i].btime);
            printf("活动结束时间:%ld\n",tmp2[i].etime);
            printf("\n");
         }
      }
   } 
   else if (type == 0) {
      return;
   }
   else {
      printf("Invalid input. Please try again\n");
   }
}

void initAct(act* myact) {
   myact->name[20] = "0";//活动名称
   myact->location[20] = "0";//活动地点
   myact->btime = 0;//开始开始时间，格式年月日小时分钟，例如：2022(年份)09（月份）05（日期）09(小时)30（分钟）其中前导0不可省略
   myact->etime = 0;//活动结束时间，格式如上
   myact->applicant = 0;//申请用户id
   myact->checkin = 0;//签到码
   myact->id = 0;//活动id，用于唯一标识一个活动(直接为出现的次序，例如第一个活动序号为1)
   myact->comment[50] = "0";//备注
   myact->auditing = -2;//活动审核情况，0为等待审核，1为审核通过，-1为被驳回
   myact->auditior = 0;//审核用户id
}

void engine_activity(int sockfd, user myuser) {
   char ch;
   act *myact;
   int response = 0;
   int type = 4;
   initAct(myact);

   // 键入活动信息
   printf("\n");
   printf("Activity name: ");
   scanf("&s", myact->name);
   ch = getchar();
   printf("Location: ");
   scanf("&s", myact->location);
   ch = getchar();
   printf("Begin time: ");
   scanf("&ld", myact->btime);
   printf("End time: ");
   scanf("&ld", myact->etime);


   write(sockfd, &type, sizeof(type));   // 向服务器发送3号请求（查询指定时间段的活动），msg = act结构体
   write(sockfd, &s_msg, sizeof(s_msg));
   while (1)
   {
      if (read(sockfd, response, sizeof(response)) > 0) {   // 等待响应
         break;
      }
   }

   if (result == 1) {
      printf("Successfully engined. Please wait administrators to audit...\n");
   }
   else if (result == 0) {
      printf("System error. Please try again.\n");
   }
   else {
      printf("Time error. Please reset the begin time and the end time.\n");
   }
}

void delete_activity(int sockfd, user myuser) {
   int mystatus = myuser.status;
   int myid = myuser.id;
   int actid = 0;
   int response = 0;
   int type = 5;
   printf("\n");
   printf("Please type in the activity ID of which you want to delete\n");
   scanf("%d", &actid);
   
   write(sockfd, &type, sizeof(type));   // 向服务器发送5号请求（删除活动），msg = 整数
   write(sockfd, &s_msg, sizeof(s_msg));
   while (1)
   {
      if (read(sockfd, response, sizeof(response)) > 0) {   // 等待响应
         break;
      }
   }

   if (response == -1) {
      printf("No Permission! (this activity isn't engined by you)\n");
   }
   else if (response == 1) {
      printf("Successfully delete.\n");
   }
   else {
      printf("Activity doesn't exist\n");
   }
}

void check_profile(user myuser) {
   printf("\n");
   printf("  Name: %s\n", myuser.name);
   printf("  ID: %s\n", myuser.id);
   printf("  Phone: %s\n", myuser.phone);
   printf("  Role: ")
   if (myuser.status == 0) {
      printf("User\n");
   }
   else if (myuser.status == 1) {
      printf("Administrator\n");
   }
   else {
      printf("Root\n");
   }
}

void delete_user(int sockfd, user myuser) {
   int deleteid;
   int response = 0;
   int type = 6;
   printf("\n");
   printf("Please type in the account you want to delete\n");
   printf("ID: ");
   scanf("%d", &deleteid);

   write(sockfd, &type, sizeof(type));   // 向服务器发送6号请求（删除账户），msg = 整数
   write(sockfd, &deleteid, sizeof(deleteid));
   while (1)
   {
      if (read(sockfd, response, sizeof(response)) > 0) {   // 等待响应
         break;
      }
   }
   response = mysend_recv(sockfd, 6, &deleteid);   
   // result = deleteuser(deleteid, myuser.status);
   
   if (result == -2 or result == -3) {
      printf("Failed to delete. (Admin account)\n");
   }
}

void check_user(int sockfd, user myuser) {
   int checkid = 0;
   int response = 0;
   int type = 7;
   user checkuser;
   printf("\n");
   printf("Please type in the id of user you want to check\n");
   printf("ID: ");
   scanf("%d", &checkid);

   write(sockfd, &type, sizeof(type));   // 向服务器发送7号请求（查询账户），msg = 整数
   write(sockfd, &checkid, sizeof(checkid));
   while (1)
   {
      if (read(sockfd, checkuser, sizeof(checkuser)) > 0) {   // 等待响应
         break;
      }
   }  
   while (1)
      {
         if (read(sockfd, &response, sizeof(response)) > 0) {   // 等待响应
            break;
         }
      }
   if (response == 0) {   // 未找到用户
      printf("Not Found.\n");
   }
   else {
      check_profile(checkuser);
   }
   search_activity(sockfd, myuser, 2);
}

void audit_activity(int sockfd, user myuser) {
   // 显示所有需要审核的活动
   int type = 8;
   act tmp[20];
   int response;
   write(sockfd, &type, sizeof(type));
   while (1)
   {
      if (read(sockfd, tmp, sizeof(tmp)) > 0) {   // 等待响应
         break;
      }
   }  
   while (1)
   {
      if (read(sockfd, &response, sizeof(response)) > 0) {   // 等待响应
         break;
      }
   }
   if (response == -1) {
      printf("No need-auditing activity exists\n");
   }
   else {
      for (int i = 0; i < response; i++) {
         printf("活动%d:\n", i + 1);
         printf("活动名称:%s\n",tmp[i].name);
         printf("活动ID:%d\n",tmp[i].id);
         printf("活动地点:%s\n",tmp[i].location);
         printf("活动开始时间:%ld\n",tmp[i].btime);
         printf("活动结束时间:%ld\n",tmp[i].etime);
         printf("活动备注：%s\n",tmp[i].comment);
         printf("\n");
      }
      printf("Choose the activity you want to audit: \n");
      int choice = 0;
      int idea = 0;
      scanf("%d", &choice);
      printf("Pass: 1     Reject: -1");
      scanf("%d", &idea);
      write(sockfd, &choice, sizeof(choice));
      write(sockfd, &idea, sizeof(idea));
   }

   int choice = 0;
   int idea;
   int audit;
   int *audit_id;

   printf("Choose the activity you want to audit: \n");
   scanf("%d", &choice);
   audit = audit_id[choice - 1];
   searchbyactid();

   printf("Audit idea(1 is accept, -1 is reject): ");
   scanf("%d", &idea);
   actauditing(AFILE, myuser.id, audit, idea);
}

void update_permission(int sockfd, user myuser) {
   int type = 9;
   int id;
   int response;
   printf("Please type in the id whose permission you want to change\n");
   scanf("%d", &id);
   write(sockfd, &type, sizeof(type));
   write(sockfd, &id, sizeof(id));
   while (1)
   {
      if (read(sockfd, &response, sizeof(response)) > 0) {   // 等待响应
         break;
      }
   }
   if (response == 1) {
      printf("Successfully change\n");
   }
   else {
      printf("ID not found\n");
   }
}
