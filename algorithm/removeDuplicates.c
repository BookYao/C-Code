
#include <stdio.h>

/*
 *  删除排序数组中的重复项
 * */
int removeDuplicates(int *num, int len)
{
    if (!num || len <= 0) {
        return 0;
    }

    int i;
    int *tmp = num;
    int ret = 1;
    for (i = 0; i < len; i++) {
        if (num[ret - 1] != tmp[i]) {
            ret++;
            num[ret - 1] = tmp[i];
        }
    }

    return ret;
}


int removeDuplicates1(int* nums, int numsSize){
    if (!nums || numsSize <= 0) {
            return 0;
    }

    int i = 1, res = 1;
    for (; i < numsSize; i++) {
        if (nums[i - 1] == nums[i]) {
            continue;
        } else {
            nums[res] = nums[i];
            res++;
        }
    }
    return res;
}


int main()
{
    //int num[] = {};
    int num[] = {1, 2, 2, 3, 4, 4, 5, 5, 6, 10, 10, 11, 12, 14, 14};
    //int num[] = {0,0,1,1,1,2,2,3,3,4};
    int ret = 0;
    //ret = removeDuplicates(num, sizeof(num)/sizeof(int));
    ret = removeDuplicates1(num, sizeof(num)/sizeof(int));
    printf("ret:%d\n", ret);
    int i;
    for (i = 0; i < ret; i++) {
        printf("num[%d]:%d\n", i, num[i]);
    }

    return 0;
}

