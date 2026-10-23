# 一个类型安全的printf

单头文件库，要求 C++14 或更新标准。`tsprintf`、`tsfprintf`、`tssprintf`、`tssnprintf`
分别为对应的标准库函数添加编译期格式检查，返回值和运行时输出行为保持一致，参数表达式只求值一次。

格式字符串必须是编译期可用的字符串；参数数量必须与格式字符串完全匹配。
格式语法参考 [cppreference 的 printf 文档](https://en.cppreference.com/cpp/io/c/fprintf)：

```text
%[flags][width][.precision][length]specifier
```

| 部分 | 支持内容 |
| --- | --- |
| 标志 | `-`、`+`、空格、`#`、`0`，允许组合和重复 |
| 宽度 | 十进制数字或 `*` |
| 精度 | `.数字`、`.*`、单独的 `.`（精度为零） |
| 整数 | `d`、`i`、`u`、`o`、`x`、`X`、`b`、`B`；支持 `hh`、`h`、`l`、`ll`、`j`、`z`、`t` |
| 浮点数 | `f`、`F`、`e`、`E`、`a`、`A`、`g`、`G`；无修饰符或 `l` 对应 `float` / `double`，`L` 对应 `long double` |
| 字符和字符串 | `c`、`s`、`lc`、`ls`；字符串支持可写和只读的对应字符指针 |
| 指针 | `p`，参数为 `void*` 或 `nullptr`；对象指针需显式转换为 `void*` |
| 写入字符计数 | `n`，支持所有整数长度修饰符，要求对应的可写有符号整数指针 |
| 百分号 | `%%`，不消耗参数，不能带标志、宽度、精度或长度修饰符 |

`*` 按宽度、精度、转换值的顺序消耗参数，宽度和精度参数经整数提升后必须为 `int`。
负宽度表示左对齐，负精度表示省略精度，由标准库处理。

整数检查考虑默认整数提升：例如 `char` / `short` 可以用于 `%d`，而 `long` 必须使用 `%ld`。
无符号转换延续原有的无符号参数限制，也允许小无符号整数的提升。
非作用域枚举按实际提升类型检查，作用域枚举和类对象需要调用者显式转换。
`j`、`z`、`t` 按当前平台的 `intmax_t`、`size_t`、`ptrdiff_t` 及对应有符号或无符号类型检查。

非法组合会在编译期被拒绝，例如 `%Ld`、`%llf`、`%#d`、`%0s`、`%.2c`、`%+n`。
不支持 POSIX 参数重排（如 `%2$d`）和平台扩展（如 `%I64d`）。

`b` 是 C++26 的二进制转换，`B` 为可选支持；本库提供其格式和类型检查，实际输出需要系统 C 库支持。
所有转换均转交标准库，平台对 `%n` 的开关限制、宽字符编码和区域设置仍然适用。

```cpp
    #include "tsprintf.hpp"

    int a = 4;
    tsprintf("a=%d in %s\n",a); //build failed, missing argument
    tsprintf("a=%d in %s\n",__FILE__,a); //build failed, mismatch type
    tsprintf("a=%d in %s\n",a++,__FILE__); //ok, and 'a' will be 5 
    tsprintf("abc%%\n"); //ok
    tsprintf("%#08x %*.*f %lld\n", 42u, 10, 3, 1.25, 42LL); //ok
    tsprintf("%-10.4ls %lc\n", L"hello", L'A'); //ok
    tsprintf("%p\n", static_cast<void*>(&a)); //ok
    tsprintf("%ld\n", a); //build failed, requires long
    tsprintf("%*f\n", 8u, 1.25); //build failed, width requires promoted int
```

运行测试需要 CMake 4.4 或更新版本；CI 使用 CMake 4.4.4 和 doctest v2.5.3：

```sh
git submodule update --init --recursive
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
ctest --test-dir build -C Debug --output-on-failure
```

CMake 项目可以通过 `tsprintf::tsprintf` 目标使用本库；设置 `-DBUILD_TESTING=OFF` 可关闭测试和 doctest 子项目。
