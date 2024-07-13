#include <stdio.h>
#include <stdlib.h>

void assert(int expected, int actual, char *code) {
  if (expected == actual) {
    printf("%s => %d\n", code, actual);
  } else {
    printf("%s => %d expected but got %d\n", code, expected, actual);
    exit(1);
  }
} 

int alloc4(int **p, int val1, int val2, int val3, int val4)
{
    *p = calloc(1,sizeof(int)* 4);
    
    **p=val1;
    *(*p+1)=val2;    
    *(*p+2)=val3;
    *(*p+3)=val4;        

    return 0;
}

void my_printf(char *str){
  printf("%s\n",str);
}