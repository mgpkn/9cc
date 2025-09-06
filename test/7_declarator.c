#include "test.h"

int main()
{

    ASSERT(24, ({ char *x[3]; sizeof(x); }));
    ASSERT(8, ({ char (*x)[3]; sizeof(x); }));
    ASSERT(8, ({ int (*x)[3]; sizeof(x); }));
    ASSERT(12, ({ char x[3][4]; sizeof(x); }));
    ASSERT(48, ({ int x[3][4]; sizeof(x); }));

    ASSERT(1, ({ char (x); sizeof(x); }));
    ASSERT(3, ({ char (x)[3]; sizeof(x); }));

    ASSERT(12, ({ char (x[3])[4]; sizeof(x); }));
    ASSERT(4, ({ char (x[3])[4]; sizeof(x[0]); }));
    ASSERT(3, ({ char *x[3]; char y; x[0]=&y; y=3; x[0][0]; }));
    ASSERT(9, ({ char x[3]; char (*y)[3]=x; y[0][0]=9; y[0][0]; }));

    return 0;
}