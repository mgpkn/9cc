#include "test.h"

int main()
{

  ASSERT(131585, (int)8590066177);
  ASSERT(513, (short)8590066177);

  ASSERT(1, (char)8590066177);
  ASSERT(-128, (char)-16909184);
  ASSERT(1, (long)1);
  ASSERT(-22136, ({int i=-305419896;(short)i; }));

  ASSERT(131585, (int)8590066177);
  ASSERT(513, (short)8590066177);
  ASSERT(1, (char)8590066177);
  ASSERT(1, (long)1);

  //ASSERT(-260, ({long i=-16909060;(short)i;})); //todo
  //ASSERT(-260, (short)-16909060); //todo
  
  ASSERT(0, (long)&*(int *)0);
  ASSERT(8, ({ int x=512; sizeof(&x); }));
  ASSERT(1, ({ int x=512; sizeof((char)&x); }));
  ASSERT(513, ({ int x=512; *(char *)&x=1; x; }));
  ASSERT(5, ({ int x=5; long y=(long)&x; *(int*)y; }));

  (void)1;

  printf("OK\n");
  return 0;
}