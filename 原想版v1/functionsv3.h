#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#define ARFILE "./udata.ar"
//用户结构体

typedef struct {
   int account;
   char *passwd;
}sign_msg;

typedef struct {
   long btime;
   long etime;
}search_msg;

// 整理时记得删除
typedef struct
{
    int status;//身份验证，0为普通用户、1为管理员、2为根用户
    int id;//用户id，用于唯一标识一个用户
    char name[20];//用户名称
    char phone[20];//用户电话
    int act[100];//该用户申请的活动，数组存放活动id，每个用户最多能申请100个活动
    char password[20];//用户密码
}user;

typedef struct
{
    char name[20];//活动名称
    char location[20];//活动地点
    long btime;//开始开始时间，格式年月日小时分钟，例如：2022(年份)09（月份）05（日期）09(小时)30（分钟）其中前导0不可省略
    long etime;//活动结束时间，格式如上
    int applicant;//申请用户id
    int checkin;//签到码
    int id;//活动id，用于唯一标识一个活动(直接为出现的次序，例如第一个活动序号为1)
    char comment[50];//备注
    int auditing;//活动审核情况，0为等待审核，1为审核通过，-1为被驳回
    int auditior;//审核用户id
}act;
//活动结构体

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

void initUser(user* myuser)
{
    for (int i = 0; i < 20; i++)
    {
        myuser->act=0;
    }
        myuser->id=0;
        myuser->name="0";
        myuser->phone="0";
        myuser->password="0";
        myuser->status=0;
    
}

void initSign_msg(sign_msg ss)
{
    ss.account=0;
    ss.passwd="0";
}
void initSearch_msg(search_msg ss)
{
    ss.btime=0;
    ss.etime=0;
}
// 删除用户函数,返回-1说明该操作账户是普通用户，无法进行删除操作，返回-2说明该操作账户是管理员账户，无法删除管理员账户，返回
// -3说明该操作账户是管理员账户，无法删除根用户账户,返回-4说明该操作账户是根用户账户，无法删除根用户账户，返回1说明操作成功
// 返回0说明发生了错误
int deleteuser(int id, int status)
{
    user user;
    FILE *fp;
    FILE *fpn;
    int ret = 1;
    if ((fpn = fopen("./tmpfile", "w")) == NULL)   // 新建一个tmp文件，赋予fpn写权限
    {
        return 0;
    }
    if ((fp = fopen(ARFILE, "r")) == NULL)   // 赋予fp读权限
    {
        return 0;
    }
    if(status==0)
    {
        ret = -1;
    }
    else if(status==1)
    {
        while (fread(&user, sizeof(user), 1, fp) == 1) // 循环复制，与输入姓名相匹配的不复制
    {
        if (user.id != id)
        {
            fwrite(&user, sizeof(user), 1, fpn); // 不相同，则复制
        }
        else if (user.status == 1)
        {

            fwrite(&user, sizeof(user), 1, fpn); // 管理员，则复制
            ret = -2;
        }
        else if(user.status==2)
        {
            fwrite(&user, sizeof(user), 1, fpn);//根用户，则复制
            ret = -3;
        }
        else {}   // 否则不复制（删除）
        memset(&user, 0x00, sizeof(user));
    }
    }
    else if(status==2)
    {
        while (fread(&user, sizeof(user), 1, fp) == 1) // 循环复制，与输入姓名相匹配的不复制
    {
        if (user.id != id)
        {
            fwrite(&user, sizeof(user), 1, fpn); // 不相同，则复制
        }
        else if(user.status==1)
        {
            fwrite(&user, sizeof(user), 1, fpn);//根用户，则复制
            ret = -4;
        }
        memset(&user, 0x00, sizeof(user));
    }
    }
    fclose(fp);
    fclose(fpn);
    remove(ARFILE);              // 删除原档案文件
    rename("./tmpfile", ARFILE); // 复制好的新文件重命名为档案文件
    return ret;
}

// 查询用户函数：返回0表示未找到用户，返回1表示找到用户。注意与searchuser_item和searchbyuser的区别
int searchuser(int id)
{
    int ret = 0;
    user user;
    FILE *fp;
    if ((fp = fopen(ARFILE, "r")) == NULL)
    {
        return 0;
    }
    while (fread(&user, sizeof(user), 1, fp) == 1)
    {
        if (user.id == id)
        {
            ret = 1;
            break;
        }
        memset(&user, 0x00, sizeof(user));
    }
    fclose(fp);
    return ret;
}

// 查询用户函数：返回user结构体
user searchuser_item(int id)
{
    user ret;
    user myuser;

    FILE *fp;
    if ((fp = fopen(ARFILE, "r")) == NULL)
    {
        return 0;
    }
    while (fread(&myuser, sizeof(myuser), 1, fp) == 1)
    {
        if (myuser.id == id)
        {
            ret = myuser;
            break;
        }
        memset(&myuser, 0x00, sizeof(myuser));
    }
    fclose(fp);

    return ret;
}

// 增加用户函数,只有注册时才能添加新账户
int adduser_login(user new)
{
    FILE *fp;
    if ((fp = fopen(ARFILE, "a")) == NULL)
    {
        return 0;
    }
    if (fwrite(&new, sizeof(new), 1, fp) < 0)
    {
        perror("fwrite");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

// 修改用户函数
int updateuser(int id)
{
    user olduser;
    FILE *fp;
    fpos_t pos;
    int found = -1;
    int cnt = 0;
    if ((fp = fopen(ARFILE, "r+")) == NULL)
    {
        return 0;
    }
        while (fread(&olduser, sizeof(olduser), 1, fp) == 1) // 循环复制，与输入id相匹配的不复制
    {
        if (olduser.id!=id)
        {
            cnt++;
        }
        else if (olduser.id==id)
        {
               if(olduser.status==1)
               {
                olduser.status=0;
               }
               else if(olduser.status==0)
               {
                olduser.status=1;
               }
               found=1;
        }
        memset(&olduser, 0x00, sizeof(olduser));
    }
    fclose(fp);
    return found;
}

// 活动查询,返回值为活动id
int searchact(act act[], int sum, int id)
{
    int i;
    for (i = 0; i < sum; i++)
    {
        if (act[i].id==id)
        {
            return i;
        }
    }
    return -1;
}

// 新增活动
int bill_insert(char *pathname, struct act *b,int status)
{
    if(status>=1)//管理员和根用户可以直接添加活动，无需审核
    {
        FILE *fp = fopen(pathname, "ab+");
        if(b->btime>=b->etime)
        {
            b->auditing=-1;
            return -1;
        }
        srand((unsigned int)time(NULL));
        b->checkin=rand()%900000+100000;//随机生成签到码
        b->auditing=1;
        b->auditior=id;
        if (fwrite(b, sizeof(struct act), 1, fp))
        {
            fclose(fp);
            return 1;
        }
        else
        {
            fclose(fp);
            return 0;
        }
    }
    else//普通用户需要审核
    {
        FILE *fp = fopen(pathname, "ab+");
        if(b->btime>=b->etime)
        {
            b->auditing=-1;
            return -1;
        }
        if (fwrite(b, sizeof(struct act), 1, fp))
        {
            fclose(fp);
            return 1;
        }
        else
        {
            fclose(fp);
            return 0;
        }
    }
}

//活动审核函数一，用于返回所有未审核的活动的结构体数组
int actauditing1(char *pathname, act *actlist)
{
    FILE *fp = fopen(pathname, "rb");
    act tmp;
    int count = 0;
    while (fread(&tmp, sizeof(struct act), 1, fp))
    {
        if(tmp.auditing==0)
        {
            actlist[count] = tmp;   // 活动添加到数组actlist
            count++;
        }
    }
    fclose(fp);
    return count;
}

void actauditing2(char *pathname, int id,int idea)
{
    FILE *fp = fopen(pathname, "rb");
    act tmp;
    int count = 0;
    while (fread(&tmp, sizeof(struct act), 1, fp))
    {
        if(tmp.id==id)
        {
            tmp.auditing=idea;   // 活动添加到数组actlist
            count++;
        }
    }
    fclose(fp);
}

// 删除活动
int bill_remove(char *pathname, int id,int status,int applicant)
{
    act a;
    FILE *fp;
    FILE *fpn;
    int ret=1;
    int flag = 0;
    if ((fpn = fopen("./tmpfile", "w")) == NULL)
    {
        return 0;
    }
    if ((fp = fopen(pathname, "r")) == NULL)
    {
        return 0;
    }
    if(status==0)//普通用户只能删除自己的活动
    {
            while (fread(&a, sizeof(a), 1, fp) == 1) // 循环复制，与输入相匹配的不复制
        {
            if (a.id!=id)
            {
             fwrite(&a, sizeof(a), 1, fpn); // 不相同，则复制
            }
            else if(a.applicant!=applicant)
            {
                flag = 1;
                fwrite(&a, sizeof(a), 1, fpn); // 申请人不为自己，则复制
                ret=-1;
            }
            memset(&a, 0x00, sizeof(a));
        }
    }
    else if(status>=1)//管理员和根用户能删除所有活动
    {
        while (fread(&a, sizeof(a), 1, fp) == 1) // 循环复制，与输入相匹配的不复制
        {
            if (a.id!=id)
            {
             fwrite(&a, sizeof(a), 1, fpn); // 不相同，则复制
            }
            memset(&a, 0x00, sizeof(a));
        }
    }
    if (flag == 0) {
        ret = 0
    }
    fclose(fp);
    fclose(fpn);
    remove(pathname);              // 删除原档案文件
    rename("./tmpfile", pathname); // 复制好的新文件重命名为档案文件
    return ret;
}

// 按照用户查询活动(用户端使用)
int searchbyuser(char *pathname, int id, act* actlist) //指用户id
{
    FILE *fp = fopen(pathname, "rb");
    act tmp;
    int count = 0;
    while (fread(&tmp, sizeof(struct act), 1, fp))
    {
        if(tmp.applicant==id) {
            actlist[count] = tmp;   // 活动添加到数组actlist
            count++;
        }
    }
    fclose(fp);
    return count;
}

// 按照时间区间查询活动
int bill_query(char *pathname, long btime,long etime,act* actlist)
{
    FILE *fp = fopen(pathname, "rb");
    act tmp;
    int count = 0;
    while (fread(&tmp, sizeof(struct act), 1, fp))
    {
        if(tmp.btime>=btime&&tmp.etime<=etime)
        {
            actlist[count] = tmp;   // 活动添加到数组actlist
            count++;
        }
    }
    fclose(fp);
    return count;
}
int decision(sign_msg ss,user *uu)//返回1说明有相应的账号密码，-1说明无
{
    FILE *fp = fopen(pathname, "rb");
    user tmp;
    while (fread(&tmp, sizeof(struct act), 1, fp))
    {
        if(ss.account==tmp.id&&strcmp(ss.passwd==tmp.password)==0) {
            uu->id=tmp.id;
            uu->status=tmp.status;
           return 1;
        }
        else if(ss.account==tmp.id&&strcmp(ss.passwd==tmp.password)!=0)
        {
            return -2;
        }
    }
    fclose(fp);
    return -1;
}