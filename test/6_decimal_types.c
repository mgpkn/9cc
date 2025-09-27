#include "test.h"

int main()
{

    //short
    short s;
    ASSERT(2,sizeof(s));    
    ASSERT(32767,({s = 32767;s;}));
    ASSERT(-32768,({s = -32768;s;}));

    //short int
    short int si;
    ASSERT(2,sizeof(si));    
    ASSERT(32767,({si = 32767;si;}));
    ASSERT(-32768,({si = -32768;si;}));

    //int
    int i;
    ASSERT(4,sizeof(i));    
    ASSERT(2147483647,({i = 2147483647;i;}));
    ASSERT(-2147483648,({i = 2147483648;i;}));    

    //i = 9223372036854775808;//must be error. (to fix)
    //ASSERT(9223372036854775808,i);//must be error. (to fix)
    
    //long
    long l;
    ASSERT(8,sizeof(l));
    ASSERT(9223372036854775808,({l = 9223372036854775808;l;}));
    ASSERT(-9223372036854775809,({l = -9223372036854775809;l;}));    

    //long int   
    long int li;
    ASSERT(8,sizeof(li));
    ASSERT(9223372036854775808,({li = 9223372036854775808;li;}));
    ASSERT(-9223372036854775809,({li = -9223372036854775809;li;}));     
    
    //long long
    long int ll;
    ASSERT(8,sizeof(ll));
    ASSERT(9223372036854775808,({li = 9223372036854775808;ll;}));
    ASSERT(-9223372036854775809,({li = -9223372036854775809;ll;}));

    //long long int
    long int lli;
    ASSERT(8,sizeof(ll));
    ASSERT(9223372036854775808,({li = 9223372036854775808;lli;}));
    ASSERT(-9223372036854775809,({li = -9223372036854775809;lli;}));   

    return 0;
}