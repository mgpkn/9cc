#include "test.h"

int main()
{

    struct mystr
    {
        int a;
        char b;
        char c;
    } x;
    x.b = 'c';
    x.c = 'a';
    x.a = 50;

    struct mystr y;
    y.a = 30;
    y.b = 'A';
    y.c = 'D';

    ASSERT(sizeof(x), 6);
    ASSERT(50, x.a);
    ASSERT(99, x.b);
    ASSERT(97, x.c);
    ASSERT(30, y.a);
    ASSERT(65, y.b);
    ASSERT(68, y.c);
    

    
    return 0;
}