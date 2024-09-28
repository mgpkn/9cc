#include "test.h"

int glob;
//int sub;

int sub()
{
    int x;
    int y;
    x=100;


    return x;
}

int main()
{
    int x;
    int y;
    x=1;
    glob=2;


    while(x<10){
        int i; //1
        i=20;
        ASSERT(20, i);                
        x = x+1;
    }

    if(x>=10){
        int i;//2
        i=13;
        ASSERT(13, i);        
        x = x+1;
    }


    ASSERT(x, x);
    ASSERT(2, glob);
    ASSERT(100, sub());
    return 0;
}