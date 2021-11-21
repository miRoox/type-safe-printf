#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include <cstdio>
#include "tsprintf.hpp"

TEST_CASE("expressions should be evaluated only once") {
    int a = 4;
    tsprintf("a=%d\n", a++);
    CHECK(a == 5);
}
