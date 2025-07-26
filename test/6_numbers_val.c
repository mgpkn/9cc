#include "test.h"

int main()
{
    int i;
    long l;

    i = 2147483647;
    ASSERT(2147483647,i);

    i = -2147483648;    
    ASSERT(-2147483648,i);    

    //i = 9223372036854775808;//must be error. (to fix)
    //ASSERT(9223372036854775808,i);//must be error. (to fix)

    l = 9223372036854775808;
    ASSERT(9223372036854775808,l);

    l = -9223372036854775809;    
    ASSERT(-9223372036854775809,l);    

    return 0;
}