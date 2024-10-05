#include "test.h"


int sum_all_param(int p1, int p2, int p3, int p4, int p5, int p6)
{
    return p1 + p2 + p3 + p4 + p5 + p6;
}

int main()
{
    int x;
    x=3;
    ASSERT(32, sum_all_param((x + 1) * x, 2, 3, 4, 5, 6));

    return 0;
}