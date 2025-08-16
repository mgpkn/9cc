#include "test.h"

int main()
{
    short s1;
    short s2;
    int i;
    long l;

    //short
    ASSERT(2,sizeof(s1));    

    s1 = 256;     
    ASSERT(256,s1);    

    s1 = 32767;     
    ASSERT(32767,s1);

    s2 = -32768;    
    ASSERT(-32768,s2);

    ASSERT(-1,s1+s2);    


    //int
    ASSERT(4,sizeof(i));    
    i = 2147483647;
    ASSERT(2147483647,i);
    i = -2147483648;    
    ASSERT(-2147483648,i);    

    //i = 9223372036854775808;//must be error. (to fix)
    //ASSERT(9223372036854775808,i);//must be error. (to fix)

    //long    
    ASSERT(8,sizeof(l));
    l = 9223372036854775808;
    ASSERT(9223372036854775808,l);
    l = -9223372036854775809;    
    ASSERT(-9223372036854775809,l);    

    return 0;
}