#include "test.h"

int main()
{


    union myuni
    {
        int a;
        char b;
        char c;
        int d;
        char *e;
    } x;


    x.a = 15;
    ASSERT(15, x.a);
    ASSERT(15, x.b);    
    ASSERT(15, x.c);
    ASSERT(15, x.d);
    ASSERT(15, x.e);         

    x.a = 410;
    ASSERT(410, x.a);
    ASSERT(-102, x.b); //singed value
    ASSERT(-102, x.c); //singed value
    ASSERT(410, x.d);    
    ASSERT(410, x.e);
  
    x.a = 5420;
    x.c = x.b + 10;
    ASSERT(5430, x.a);
    ASSERT(54, x.b);
    ASSERT(54, x.c);
    ASSERT(5430, x.d);
    ASSERT(5430, x.e);       

    //calc sizeof
    ASSERT(4, sizeof(x.a));
    ASSERT(1, sizeof(x.b));
    ASSERT(1,sizeof(x.c));
    ASSERT(4, sizeof(x.d)); 
    ASSERT(8, sizeof(x.e));      

    //ASSERT(99, x.b);
    //ASSERT(97, x.c);
    
    return 0;
}