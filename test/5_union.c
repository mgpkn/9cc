#include "test.h"

int main()
{


    union myuni
    {
        int a;
        char b;
        char c;
        int d;
    } x;


    x.a = 15;
    ASSERT(15, x.a);
    ASSERT(15, x.b);    
    ASSERT(15, x.c);
    ASSERT(15, x.d);     

    x.a = 410;
    ASSERT(410, x.a);
    ASSERT(-102, x.b); //singed value
    ASSERT(-102, x.c); //singed value
    ASSERT(410, x.d);    

  
    x.a = 5420;
    x.c = x.b + 10;
    ASSERT(5430, x.a);
    ASSERT(54, x.b);
    ASSERT(54, x.c);
    ASSERT(5430, x.d);   

    //ASSERT(99, x.b);
    //ASSERT(97, x.c);
    
    return 0;
}