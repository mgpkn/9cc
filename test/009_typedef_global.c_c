#include "test.h"

typedef char gmyint;
typedef gmyint2;
typedef int *gmyint3,gmyint4[3];
int main()
{
    ASSERT(16, ({ gmyint x; sizeof(x); }));
    ASSERT(65, ({ gmyint x; x='A';x; }));
    ASSERT(4, ({ gmyint2 x; sizeof(x); }));
    ASSERT(88, ({ gmyint2 x; x=88;x; }));
    ASSERT(8, ({ gmyint3 x; sizeof(x); }));
    ASSERT(12, ({ gmyint4 x; sizeof(x); }));
    return 0;
}