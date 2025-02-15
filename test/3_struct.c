#include "test.h"

int main()
{


    struct {int a;char b;char c;} st;
    st.b = 'c';
    st.c = 'a';
    st.a = 50;

    ASSERT(sizeof(st),6);
    ASSERT(50,st.a);    
    ASSERT(99,st.b);    
    ASSERT(97,st.c);    

    return 0;
}