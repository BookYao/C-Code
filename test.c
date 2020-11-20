#include <stdio.h>
#include <stdlib.h>
#include "list_head.h"
#include "stu.h"

int main(int argc, const char *argv[])
{

    int i;
    struct STU *stu;
    struct STU *stup;
    struct list_head *pos;
    struct list_head stu_list;//学生表的首节点

    stu=(struct STU *)malloc(sizeof(struct STU)*5);  //分配节点空间
    if(!stu)
    {
        perror("fail to malloc");
        return -1;
    }

    //初始化通用链表
    INIT_LIST_HEAD(&stu_list);
    for(i=0;i<5;i++)  //赋值学生信息
    {
        stu[i].id=i;
        sprintf(stu[i].name,"stu%d",i);
        sprintf(stu[i].pwd,"160%d",i);
        stu[i].score=10*i;
        list_add(&stu[i].list,&stu_list);//插入链表
    }

    //遍历学生信息
    list_for_next_each(pos,&stu_list)
    {
        stup=container_of(pos,struct STU,list);
        printf("stu:id=%d,name=%s,pwd=%s,score=%f\n",\
                stup->id,stup->name,stup->pwd,stup->score);
    }

    puts("--------------------------------");

    list_for_prev_each(pos,&stu_list)
    {
        stup=container_of(pos,struct STU,list);
        printf("stu:id=%d,name=%s,pwd=%s,score=%f\n",\
                stup->id,stup->name,stup->pwd,stup->score);
    }

    return 0;
}

