#include "test.h"

int main()
{

    //tag struct with declare tag.
    struct mystr
    {
        int a;
        char b;
        char c;
    } x;
    x.b = 'c';
    x.c = 'a';
    x.a = 50;

    //struct called by tag.
    struct mystr x2;
    x2.a = 30;
    x2.b = 'A';
    x2.c = 'D';


    //non tag struct.
    struct 
    {
        int a;
        char b;
        char c;
    } x3;
    x3.b = 'd';
    x3.c = 'b';
    x3.a = 51;

    // tag duplicate error.
    /*
    struct mystr
    {
        int a;
        char b;
        char c;
    } x5;
    */

    // undfined tag error.
    //struct undefined_mystr x6;

    //member
    ASSERT(sizeof(x), 8);
    ASSERT(50, x.a);
    ASSERT(99, x.b);
    ASSERT(97, x.c);

    ASSERT(30, x2.a);
    ASSERT(65, x2.b);
    ASSERT(68, x2.c);

    ASSERT(sizeof(x3), 8);
    ASSERT(51, x3.a);
    ASSERT(100, x3.b);
    ASSERT(98, x3.c);

    //arrow operator
    struct mystr *z;
    z = &x2;
    ASSERT(30, z->a);
    ASSERT('A', z->b);    
    
    return 0;
}