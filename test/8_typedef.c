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

    //same name.(typedef and variable)
    ASSERT(63, ({ typedef int t; t t=63; t; }));

    //struct and union
    ASSERT(1, ({ typedef struct {int a;} s; s x; x.a=1; x.a; }));
    ASSERT(97, ({ typedef union {int a;char b;} u; u x; x.b='a'; x.b; }));
    
    //meaningless type def.
    typedef int;

    //multi name typedef.
    typedef int MyInt, MyInt2[4],*MyInt3; 

    ASSERT(1, ({ typedef int t; t x=1; x; }));
    ASSERT(1, ({ typedef struct {int a;} t; t x; x.a=1; x.a; }));
    ASSERT(1, ({ typedef int t; t t=1; t; }));
    ASSERT(3, ({ MyInt x=3; x; }));
    ASSERT(16, ({ MyInt2 x; sizeof(x); }));
    ASSERT(8 , ({ MyInt3 x; sizeof(x); }));
    ASSERT(4, ({ typedef t; t x; sizeof(x); }));
    
    ASSERT(12, ({ typedef struct {int a;int b;int c;} t; { typedef char t; } t x; sizeof(x); }));
    ASSERT(88, ({ typedef struct {int a;int b;int c;} t; { typedef char t; } t x; x.c=88;x.c; }));
    ASSERT(1, ({ typedef char t;{typedef struct {int a;int b;int c;} t;}t x; sizeof(x);}));

    return 0;
}