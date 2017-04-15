# 一个类型安全的printf

目前只支持诸如"%d","%s"这样的基本格式

```cpp
    tsprintf("a=%d in %s\n",a); //wrong 
    tsprintf("a=%d in %s\n",__FILE__,a); //wrong
	tsprintf("a=%d in %s\n",a++,__FILE__); //ok, and 'a' will be 5 
    tsprintf("abc%%\n"); //ok
```
