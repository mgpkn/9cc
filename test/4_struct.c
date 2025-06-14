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

    //member
    ASSERT(sizeof(x), 8);
    ASSERT(50, x.a);
    ASSERT(99, x.b);
    ASSERT(97, x.c);

    //tag
    struct mystr y;
    y.a = 30;
    y.b = 'A';
    y.c = 'D';

    ASSERT(30, y.a);
    ASSERT(65, y.b);
    ASSERT(68, y.c);

    //arrow operator
    struct mystr *z;
    z = &y;
    ASSERT(30, z->a);
    ASSERT('A', z->b);    
    
    return 0;
}