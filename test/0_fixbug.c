#include "test.h"

int main()
{

    int y=10;
    struct mystr
    {
        char b;
        char c;
        int a;        
    } x;
    x.a = 50;    
    x.b = 'c';
    x.c = 'a';


    ASSERT(10,y);
    ASSERT(50,x.a);
    ASSERT('c',x.b);
    ASSERT('a',x.c);


    return 0;
}