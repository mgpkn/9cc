#include "test.h"

int ret3()
{
    return 3;
    return 5;
}

int myadd(int a, int b)
{
    return a + b;
}

int foo()
{
    return 4;
}

int baa()
{
    return 2;
}

int aba(int d)
{
    return d;
}

int myadd2(int a, int b)
{
    return a + b;
}

int ret_arg2(int a, int b,int c,int d,int e)
{
    return b;
}

int sum_all_param(int p1, int p2, int p3, int p4, int p5, int p6)
{
    return (p1*2) + p2 + p3 + p4 + p5 + p6;
}

int araara()
{
    int i;
    int j;
    j=10;
    for(i=1;i <= 1;i=i+1)
    {
        j=j+1;
    }
    return i+j;
}

int ufufu()
{
    int fuga;
    fuga = 1;
    while (fuga < 7)
    {
        fuga = fuga + 1;
    }
    return fuga;
}

int glob;
int *glob_addr;

int main()
{
    my_printf("##start##\n");

    int x;
    int *y;
    int **z;
    int arr[10];

    ASSERT(5, 9 + -4);
    ASSERT(3, ret3());
    ASSERT(0, 9 + -9);
    ASSERT(8, +11 + 9 + 8 - 4 - 4 + -12);
    ASSERT(23, 11 + 3 * 4);
    ASSERT(-1, -10 + 9);

    x = (3 + 3) * 5;
    ASSERT(30, x);

    ASSERT(23, 11 + 3 * 4);


    ASSERT(1, 100 < 101);
    ASSERT(0, 100 < 100);
    ASSERT(1, 99 <= 100);
    ASSERT(1, 100 <= 100);
    ASSERT(1, 100 <= 101);
    ASSERT(1, 101 > 100);
    ASSERT(0, 101 > 101);
    ASSERT(1, 100 >= 99);
    ASSERT(1, 100 >= 100);
    ASSERT(0, 100 >= 101);

    ASSERT(13, araara());
    ASSERT(7, ufufu());
    x = 3;
    ASSERT(44, sum_all_param((x + 1) * x, 2, 3, 4, 5, 6));

    if (0)
        x = 13;
    else
        x = 8;
    ASSERT(8, x);

    if(100)x=13; else x=8;
    ASSERT(13,x);

    x=1;
    while(x<8){x=x+1;}    
    ASSERT(7, ufufu());

    glob = 15;
    ASSERT(150, glob * 10);

    ASSERT(32, ret_arg2(100,32,48,3,58));

    y = &x;
    *y = 32;
    ASSERT(32, x);

    y = &x;
    int *y2;
    //y2 = &y+32; todo
    z = &y;
    **z = 13;
    ASSERT(13, x);
    //ASSERT(32,y2 - &y); todo

    arr[0]=22;
    arr[1]=13;

    ASSERT(13, arr[1]);
    ASSERT(13, *(arr+1));    
    ASSERT(35, arr[0]+arr[1]);    

    alloc4(&y,13,22,35,48);

    ASSERT(22,*(y+1));    
    ASSERT(49,*(y+3)+1);
    ASSERT(1,(z+2)-(z+1));    


    ASSERT(4, sizeof(x));
    ASSERT(4, sizeof(x + 100));
    ASSERT(8, sizeof(y));
    ASSERT(8, sizeof(&x));
    ASSERT(40, sizeof(arr));
    ASSERT(4, sizeof(arr[1]));
    ASSERT(1, sizeof('a'));    
    //ASSERT(4, sizeof("abc"));  //todo
    //ASSERT(1, sizeof("abc"[0]));  //todo

    arr[0]=13;
    arr[1]=22;    

    x=2;glob=20;
    ASSERT(40,glob*x);

    x=55;
    glob_addr=&x;
    ASSERT(55,*glob_addr);

    y=y+3;
    ASSERT(48,*y);
    ASSERT(22,*(y-2));

    ASSERT(3, myadd(1, 2));
    ASSERT(50, aba(50));
    ASSERT(13, myadd(3, 10));
    
    //declare and initialize 
    int xx=123*2;

    ASSERT(246, xx);

    char c_ini = 'A';
    ASSERT(c_ini, 'A');

    ASSERT(0,({ 0; }));
    ASSERT(12,({ 0; 1; 2*6; }));    
    ASSERT(18,({ x; x = 3; x*6;}));
    ASSERT(6,({ 1; }) + ({ 2; }) + ({ 3; }));        
    ASSERT(10,({ 1;5; }) + ({ 2; }) + ({ 3; }));
    //assert 1 'int main() { ({ 0; return 1; 2; }); return 3; }' //todo
    ASSERT(18,({x; x = 3;18;}));
    ASSERT(24,({x; x = 3;18+3*2;}));

    ASSERT(24,({x; x = 3;18+3*2;}));    

    //comma oparator
    x=(3,10,305);
    ASSERT(305,x);    

    my_printf("##end##\n");

    return 0;
}