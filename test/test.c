#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>

void alloc4(int **p, int val1, int val2, int val3, int val4)
{
    *p = calloc(1,sizeof(int)* 4);
    
    **p=val1;
    *(*p+1)=val2;    
    *(*p+2)=val3;
    *(*p+3)=val4;        

}

int sum_all_param(int p1,int p2,int p3,int p4,int p5,int p6){
  return p1+p2+p3+p4+p5+p6;
}

int main()
{
    return sum_all_param(1,2,3,4,5,6);
}
