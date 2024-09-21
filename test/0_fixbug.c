#include "test.h"

int glob;
int sub;

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

/*
    while(x<10){
        int i;
        x = x+1;
    }

    while(x<10){
        int i;
        i=10;
        x = x+1;
    }
*/

    ASSERT(1, x);
    ASSERT(2, glob);
    ASSERT(100, sub());
    return 0;
}