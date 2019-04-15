/*
 * util.c
 *
 *  Created on: Apr 14, 2019
 *      Author: Administrator
 */
#include"algebra.h"

u32 FakeRand(int n)
{
    u32 x = 0;

    for(int i=0; i<20; i++)
    {
        x ^= n<<i;
    }
    x++;
    return x;
}

void memout(u8 *pdata,int len)
{
    int i;
    for(i=0;i<len;i++)
    {
        printf("%d ",*(pdata+i));
        if((i+1)%8==0)
        {
            printf("\n");
        }
    }
    printf("\n");
}
