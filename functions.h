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

// 增加用户函数,返回值为-1说明该操作账户是普通用户，无法进行增加操作,返回值为-2说明该操作账户是管理员用户，无法进行增加管理员
// 或根用户操作,回值为-3说明该操作账户是根用户，无法进行增加根用户操作，返回值为0说明出现错误，返回值为1说明成功
int adduser(user new,int status)
{
    FILE *fp;
    if ((fp = fopen(ARFILE, "a")) == NULL)
    {
        return 0;
    }
    if(status==0)
    {
        return -1;
    }
    else if(status==1&&new.status>=1)
    {
        return -2;
    }
    else if(status==2&&new.status==2)
    {
        return -3;
    }
    else if((status==1&&new.status==0)||(status==2&&new.status<=1))
    {if (fwrite(&new, sizeof(new), 1, fp) < 0)
    {
        perror("fwrite");
        fclose(fp);
        return 0;
    }
    }
    fclose(fp);
    return 1;
}

// 修改用户函数
int updateuser(user newuser,int status)
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
    if(status==0)//普通用户
    {
        found=-2;
    }
    else if(status==1&&olduser.status>=1)//管理员想修改管理员或根用户账户
    {
        found=-3;
    }
    else if(status==1&&olduser.status==0&&newuser.status>=1)//管理员想将普通用户修改为管理员或根用户
    {
        found=-4;
    }
    else if(status==2&&newuser.status==2&&olduser.status<=1)//根用户想将普通用户或管理员修改为根用户
    {
        found=-5;
    }
    else if(olduser.status==2)//任意用户想修改根用户
    {
        found=-6;
    }
    else if((status==1&&newuser.status==0&&olduser.status==0)||(status==2&&newuser.status<=1&&olduser.status<=1))
    {
        while (fread(&olduser, sizeof(olduser), 1, fp) == 1) // 循环复制，与输入id相匹配的不复制
    {
        if (olduser.id!=newuser.id)
        {
            cnt++;
        }
        else if (olduser.id==newuser.id)
        {
                found = 1;
                pos.__pos = (long)(cnt * sizeof(struct user)); // 设置移动量为一个结构体
                fsetpos(fp, &pos);                                   // 另外一种移动文件指针位置的方法
                fwrite(&newuser, sizeof(newuser), 1, fp);
        }
        memset(&olduser, 0x00, sizeof(olduser));
    }
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

//活动审核函数
int actauditing(char *pathname, int id, int i,int idea)//第二个参数是审核用户id，第三个参数是活动id，第四个参数是审核意见(1或者-1)
{
    FILE *fp;
    act tmp;
    fp = fopen(pathname, "r+b");
    fseek(fp, i * sizeof(struct act), SEEK_SET);
    fread(&tmp, sizeof(tmp), 1, fp);
    fseek(fp, i * sizeof(struct act), SEEK_SET);
    if (tmp.auditing)
    {
        fclose(fp);
        return 0;
    }
    else
    {
        tmp.auditing = idea;
        tmp.auditior=id;
        fwrite(&tmp, sizeof(tmp), 1, fp);
        fclose(fp);
        return 1;
    }
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
int searchbyuser(char *pathname, int id， act* actlist) //指用户id
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
void int bill_query(char *pathname, long btime,long etime)
{
    FILE *fp = fopen(pathname, "rb");
    act tmp;
    int count = 0;
    while (fread(&tmp, sizeof(struct act), 1, fp))
    {
        if(tmp.btime>=btime&&tmp.etime<=etime)
        {
            count++;
            printf("活动%d:\n",count);
            printf("活动名称:%s\n",tmp.name);
            printf("活动地点:%s\n",tmp.location);
            printf("活动开始时间:%ld\n",tmp.btime);
            printf("活动结束时间:%ld\n",tmp.etime);
            printf("活动备注：%s\n",tmp.comment);
            if(tmp.auditing==0)
            {printf("活动审核情况:审核中\n");}
            if(tmp.auditing==1)
            {printf("活动审核情况:审核通过\n");}
            if(tmp.auditing==-1)
            {printf("活动审核情况:驳回\n");}
            printf("审核人id:%d\n",tmp.auditior);
            printf("签到码:%d\n",tmp.checkin);
            printf("\n");        }
    }
    fclose(fp);
    printf("\n");
}
