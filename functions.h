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

#ifndef ARFILE
#define ARFILE "./user.ar" // 指定档案文件的路径名称
#endif
#define BRFILE "./act.ar"
struct myuser
{
    int status;//身份验证，0为普通用户、1为管理员、2为根用户
    int id;//用户id，用于唯一标识一个用户
    char name[20];//用户名称
    char phone[20];//用户电话
    int act[100];//该用户申请的活动，数组存放活动id，每个用户最多能申请100个活动
    char password[20];//用户密码
};

struct act
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
};

// 物品查询
int search(char *pathname, int id)
{
    FILE *fp = fopen(pathname, "r+b");
    struct act tmp;
    int found = 0;
    int cnt = 0;
    while (fread(&tmp, sizeof(tmp), 1, fp))
    {
        if (tmp.id==id)
        {
            found = 1;
            break;
        }
        cnt++;
    }
    fclose(fp);
    if (found)
    {
        return cnt;
    }
    else
    {
        return -1;
    }
}

struct myuser searchus(char *pathname, int id)
{
    FILE *fp = fopen(pathname, "rb");
    struct myuser tmp;
    int found = 0;
    int cnt = 0;
    while (fread(&tmp, sizeof(tmp), 1, fp))
    {
        if (tmp.id==id)
        {
            return tmp;
            break;
        }
        cnt++;
    }
    fclose(fp);
    return tmp;
}

// 新增物品
int actinsert(struct act a)
{

    FILE *fp;
    if ((fp = fopen(BRFILE, "a")) == NULL)
    {
        return 0;
    }
    if (fwrite(&a, sizeof(a), 1, fp) < 0)
    {
        perror("fwrite");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;

}

// 删除物品
int actremove(char *pathname, int id)
{
    struct act a;
    FILE *fp;
    FILE *fpn;
    if ((fpn = fopen("./tmpfile", "w")) == NULL)
    {
        return 0;
    }
    if ((fp = fopen(pathname, "r")) == NULL)
    {
        return 0;
    }
    while (fread(&a, sizeof(a), 1, fp) == 1) // 循环复制，与输入相匹配的不复制
    {
        if (a.id==id)
        {
            fwrite(&a, sizeof(a), 1, fpn); // 不相同，则复制
        }
        memset(&a, 0x00, sizeof(a));
    }
    fclose(fp);
    fclose(fpn);
    remove(pathname);              // 删除原档案文件
    rename("./tmpfile", pathname); // 复制好的新文件重命名为档案文件
    return 1;
}

int actupdate(char *pathname,int id,int i)
{
    FILE *fp;
    struct act tmp;
    fp = fopen(pathname, "r+b");
    fseek(fp, i * sizeof(struct act), SEEK_SET);
    fread(&tmp, sizeof(struct act), 1, fp);
    fseek(fp, i * sizeof(struct act), SEEK_SET);
    if (tmp.auditing)
    {
        fclose(fp);
        return 0;
    }
    else
    {
        tmp.auditing = 1;
        tmp.auditior=id;
        fwrite(&tmp, sizeof(tmp), 1, fp);
        fclose(fp);
        return 1;
    }
}

int removeuser(char *name)
{
    struct myuser user;
    FILE *fp;
    FILE *fpn;
    int ret = 1;
    if ((fpn = fopen("./tmpfile", "w")) == NULL)
    {
        return 0;
    }
    if ((fp = fopen(ARFILE, "r")) == NULL)
    {
        return 0;
    }
    while (fread(&user, sizeof(user), 1, fp) == 1) // 循环复制，与输入姓名相匹配的不复制
    {
        if (strcmp(user.name, name) != 0)
        {
            fwrite(&user, sizeof(user), 1, fpn); // 不相同，则复制
        }
        else if (user.status == 1)
        {
            fwrite(&user, sizeof(user), 1, fpn); // 管理员，则复制
            ret = -1;
        }
        memset(&user, 0x00, sizeof(user));
    }
    fclose(fp);
    fclose(fpn);
    remove(ARFILE);              // 删除原档案文件
    rename("./tmpfile", ARFILE); // 复制好的新文件重命名为档案文件
    return ret;
}

// 查询员工函数
int queryuser(char *name)
{
    int found = 0;
    struct myuser user;
    FILE *fp;
    if ((fp = fopen(ARFILE, "r")) == NULL)
    {
        return 0;
    }
    while (fread(&user, sizeof(user), 1, fp) == 1)
    {
        if (strcmp(user.name,name)==0)
        {
            found = 1;
            break;
        }
        memset(&user, 0x00, sizeof(user));
    }
    fclose(fp);
    return found;
}

// 增加用户函数
int insertuser(struct myuser uu)
{
    FILE *fp;
    if ((fp = fopen(ARFILE, "a")) == NULL)
    {
        return 0;
    }
    if (fwrite(&uu, sizeof(uu), 1, fp) < 0)
    {
        perror("fwrite");
        fclose(fp);
        return 0;
    }
    fclose(fp);
    return 1;
}

// 修改用户函数
int updateuser(struct myuser newuser)
{
    struct myuser olduser;
    FILE *fp;
    fpos_t pos;
    int found = -1;
    int cnt = 0;
    if ((fp = fopen(ARFILE, "r+")) == NULL)
    {
        return 0;
    }
    while (fread(&olduser, sizeof(olduser), 1, fp) == 1) // 循环复制，与输入姓名相匹配的不复制
    {
        if (olduser.id!=newuser.id)
        {
            cnt++;
        }
        else if (olduser.id==newuser.id)
        {
            if (olduser.status==1)
            {
                return -2;
            }
            else
            {
                found = 1;
                pos.__pos = (long)(cnt * sizeof(struct myuser)); // 设置移动量为一个结构体
                fsetpos(fp, &pos);                                   // 另外一种移动文件指针位置的方法
                fwrite(&newuser, sizeof(newuser), 1, fp);
            }
        }
        memset(&olduser, 0x00, sizeof(olduser));
    }
    return found;
}