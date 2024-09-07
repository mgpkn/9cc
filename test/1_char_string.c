#include "test.h"

int main()
{
    
    char c1;
    char ch2;
    char ch3;
    char ch4;
    char ch5;
    char ch6;
    char char7;    

    c1='H';
    ch2=101;       
    ch3='l';
    ch4='\154';
    ch5='o';
    ch6='\n'; 
    char7='\x1a';         

    ASSERT(72, c1);
    ASSERT(101,ch2);    
    ASSERT(108,ch3);    
    ASSERT(108,ch4);
    ASSERT(111,ch5);
    ASSERT(10, ch6);
    ASSERT(26, char7);    

    my_printf("\x48\x65\x78\x5F\x44\x61\x74\x61\n");
    my_printf("##end##\n");


    return 0;
}