#include "test.h"

int main()
{
    typedef int myint,*myint2,myint3[5];

    //basic
    ASSERT(12,({myint i;i=12;}));
    ASSERT(4,({myint i;sizeof(i);}));

    //pointer
    int j=30;
    myint2 i2;
    i2=&j;
    ASSERT(30,*i2);
    ASSERT(8,sizeof(i2));

    //list
    myint3 i3;
    i3[4]=11;
    ASSERT(11,i3[4]);
    ASSERT(20,sizeof(i3));


    return 0;
}