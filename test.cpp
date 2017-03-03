#include "tsprintf.hpp"

int main()
{
    int a = 4;
    tsprintf("a=%d in %s",a,__FILE__);

    tsprintf("abc%%");
/*
    tsprintf("a=%d in %s",a);

    tsprintf("a=%d in %s",__FILE__,a);
*/
    return 0;
}
