#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "tsprintf.hpp"

TEST_CASE("percentage escape") {
    constexpr bool isvalid = _tsprintf_check_fmt("50%%");
    CHECK(isvalid);
}
TEST_CASE("hanging percentage") {
    constexpr bool isvalid = _tsprintf_check_fmt("50%");
    CHECK_FALSE(isvalid);
}
TEST_CASE_TEMPLATE("single int:", T, signed char, short, int) {
    T a = 1;
    SUBCASE("%d") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%d", a);
        CHECK(isvalid);
    }
    SUBCASE("%i") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%i", a);
        CHECK(isvalid);
    }
    SUBCASE("%u") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%u", a);
        CHECK_FALSE(isvalid);
    }
    SUBCASE("%o") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%o", a);
        CHECK_FALSE(isvalid);
    }
    SUBCASE("%x") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%x", a);
        CHECK_FALSE(isvalid);
    }
    SUBCASE("%X") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%X", a);
        CHECK_FALSE(isvalid);
    }
}
TEST_CASE_TEMPLATE("single unsigned:", T, unsigned char, unsigned short, unsigned int) {
    T a = 1;
    SUBCASE("%u") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%u", a);
        CHECK(isvalid);
    }
    SUBCASE("%o") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%o", a);
        CHECK(isvalid);
    }
    SUBCASE("%x") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%x", a);
        CHECK(isvalid);
    }
    SUBCASE("%X") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%X", a);
        CHECK(isvalid);
    }
}
TEST_CASE_TEMPLATE("single float:", T, float, double) {
    T a = 1;
    SUBCASE("%f") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%f", a);
        CHECK(isvalid);
    }
    SUBCASE("%e") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%e", a);
        CHECK(isvalid);
    }
    SUBCASE("%a") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%a", a);
        CHECK(isvalid);
    }
    SUBCASE("%g") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%g", a);
        CHECK(isvalid);
    }
    SUBCASE("%F") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%F", a);
        CHECK(isvalid);
    }
    SUBCASE("%F") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%E", a);
        CHECK(isvalid);
    }
    SUBCASE("%A") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%A", a);
        CHECK(isvalid);
    }
    SUBCASE("%G") {
        constexpr bool isvalid = _tsprintf_check_fmt("a=%G", a);
        CHECK(isvalid);
    }
}
TEST_CASE("single int pointer") {
    int n = 1;
    SUBCASE("%n") {
        constexpr bool isvalid = _tsprintf_check_fmt("count: %n", &n);
        CHECK(isvalid);
    }
    SUBCASE("%p") {
        constexpr bool isvalid = _tsprintf_check_fmt("address: %p", &n);
        CHECK(isvalid);
    }
}

TEST_CASE("expressions should be evaluated only once") {
    int a = 4;
    tsprintf("a=%d\n", a++);
    CHECK(a == 5);
}
