#include "tsprintf.hpp"

int main()
{
    int a = 4;
#ifdef ok
    tsprintf("a=%d in %s\n",a++,__FILE__);

    if(a==5)
        tsprintf("abc%%\n");

#else
    tsprintf("a=%d in %s\n",a);

    tsprintf("a=%d in %s\n",__FILE__,a);

#endif
    return 0;
}
