#include "test.h"

int main()
{
    ASSERT(6,({ 1; }) + ({ 2; }) + ({ 3; }));

    return 0;
}