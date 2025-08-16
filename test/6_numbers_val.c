#include "test.h"

int main()
{
    short s1;
    short s2;
    int i;
    long l;

    //short
    ASSERT(2,sizeof(s1));    
    ASSERT(32767,({s1 = 32767;s1;}));
    ASSERT(-32768,({s2 = -32768;s2;}));

    //int
    ASSERT(4,sizeof(i));    
    
    ASSERT(2147483647,({i = 2147483647;i;}));
    ASSERT(-2147483648,({i = 2147483648;i;}));    

    //i = 9223372036854775808;//must be error. (to fix)
    //ASSERT(9223372036854775808,i);//must be error. (to fix)

    //long    
    ASSERT(8,sizeof(l));
    
    ASSERT(9223372036854775808,({l = 9223372036854775808;l;}));
    ASSERT(-9223372036854775809,({l = -9223372036854775809;l;}));    

    return 0;
}