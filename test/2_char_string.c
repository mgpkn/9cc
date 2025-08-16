#include "test.h"

int main()
{
    
    ASSERT(1, ({char c;sizeof(c);}));
    ASSERT(8, ({char *c;sizeof(c);}));    
    ASSERT(72, ({char c1;c1='H';}));
    ASSERT(101,({char ch2;ch2=101;}));    
    ASSERT(108,({char ch3;ch3='l';}));    
    ASSERT(108,({char ch4;ch4='\154';}));
    ASSERT(111,({char ch5;ch5='o';}));
    ASSERT(10, ({char ch6;ch6='\n'; }));
    ASSERT(26, ({char char7;char7='\x1a';}));    

    my_printf("\x48\x65\x78\x5F\x44\x61\x74\x61\x5f\n");
    my_printf("##end##\n");
    
    return 0;
}