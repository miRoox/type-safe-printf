# һ�����Ͱ�ȫ��printf

Ŀǰֻ֧������"%d","%s"�����Ļ�����ʽ

```cpp
    tsprintf("a=%d in %s\n",a); //wrong 
    tsprintf("a=%d in %s\n",__FILE__,a); //wrong
	tsprintf("a=%d in %s\n",a++,__FILE__); //ok, and 'a' will be 5 
    tsprintf("abc%%\n"); //ok
```
