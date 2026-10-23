#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "tsprintf.hpp"
#include <cstring>
#include <string>

// Every parser assertion must also be a C++14 constant expression.
#define CHECK_VALID_FORMAT(...) do { \
    constexpr bool valid = _tsprintf_check_fmt(__VA_ARGS__); \
    CHECK(valid); \
} while(false)

#define CHECK_INVALID_FORMAT(...) do { \
    constexpr bool valid = _tsprintf_check_fmt(__VA_ARGS__); \
    CHECK_FALSE(valid); \
} while(false)

#define CHECK_INTEGER_FORMATS(Signed, Unsigned, length) do { \
    Signed signedValue = 1; \
    Unsigned unsignedValue = 1; \
    CHECK_VALID_FORMAT("%" length "d", signedValue); \
    CHECK_VALID_FORMAT("%" length "i", signedValue); \
    CHECK_VALID_FORMAT("%" length "u", unsignedValue); \
    CHECK_VALID_FORMAT("%" length "o", unsignedValue); \
    CHECK_VALID_FORMAT("%" length "x", unsignedValue); \
    CHECK_VALID_FORMAT("%" length "X", unsignedValue); \
    CHECK_VALID_FORMAT("%" length "b", unsignedValue); \
    CHECK_VALID_FORMAT("%" length "B", unsignedValue); \
} while(false)

template<typename Parser, std::size_t N>
constexpr bool allFormatsInvalid(const char* const (&formats)[N])
{
    for(auto fmt : formats) {
        if(Parser::checkFormat(fmt)) {
            return false;
        }
    }
    return true;
}

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
    SUBCASE("%E") {
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
        constexpr bool isvalid = _tsprintf_check_fmt("address: %p", static_cast<void*>(&n));
        CHECK(isvalid);
    }
}

TEST_CASE("expressions should be evaluated only once") {
    int a = 4;
    tsprintf("a=%d\n", a++);
    CHECK(a == 5);
}

TEST_CASE("flags, constant width and precision") {
    CHECK_VALID_FORMAT("%+- 08.4d", 42);
    CHECK_VALID_FORMAT("%#012.8x", 42u);
    CHECK_VALID_FORMAT("%#o", 42u);
    CHECK_VALID_FORMAT("%+#016.3A", 1.5);
    CHECK_VALID_FORMAT("%-10.4s", "hello");
    CHECK_VALID_FORMAT("%5c", 'x');
    CHECK_VALID_FORMAT("%12p", static_cast<void*>(nullptr));
    CHECK_VALID_FORMAT("%.d %.f %.s", 0, 1.5, "hello");
    CHECK_VALID_FORMAT("%--++  0008d", 1); // Repeated flags are permitted.
    CHECK_VALID_FORMAT("%% %04d%%%s %%", 1, "x");
    CHECK_VALID_FORMAT("%d%d%s", 1, 2, "x");
    CHECK_VALID_FORMAT("%%a%%b%%%%");
    CHECK_VALID_FORMAT("");
}

TEST_CASE("dynamic width and precision consume promoted int arguments in order") {
    CHECK_VALID_FORMAT("%*d", 8, 42);
    CHECK_VALID_FORMAT("%.*f", 3, 1.5);
    CHECK_VALID_FORMAT("%*.*f", -8, -1, 1.5);
    CHECK_VALID_FORMAT("%#*.*llx %.*ls %d", 12, 8, 42ull, 3, L"wide", 5);
    CHECK_VALID_FORMAT("%*c/%.*s/%*.*f", 3, 'x', 2, "hello", 8, 2, 1.5);
    CHECK_VALID_FORMAT("%*.*f", short(8), static_cast<signed char>(2), 1.5f);

    CHECK_INVALID_FORMAT("%*d", 8);
    CHECK_INVALID_FORMAT("%.*s", "text");
    CHECK_INVALID_FORMAT("%*.*f", 8, 2);
    CHECK_INVALID_FORMAT("%*.*f", 8, 1.5, 2);
    CHECK_INVALID_FORMAT("%*f", 8u, 1.5);
    CHECK_INVALID_FORMAT("%.*f", 2L, 1.5);
    CHECK_INVALID_FORMAT("%*.*f", 8, 2.0, 1.5);
    CHECK_INVALID_FORMAT("%*d", 8, 42, 1);
    CHECK_INVALID_FORMAT("%d %*.*f", 1, 8, 2, 1.5, 4);
    CHECK_INVALID_FORMAT("%%", 8);
    CHECK_INVALID_FORMAT("%d", 1, 2);
    CHECK_INVALID_FORMAT("%d");
    CHECK_INVALID_FORMAT("%*.*f");
}

TEST_CASE("all integer conversions and length modifiers") {
    using signed_size = typename std::make_signed<std::size_t>::type;
    using unsigned_difference = typename std::make_unsigned<std::ptrdiff_t>::type;
    CHECK_INTEGER_FORMATS(int, unsigned int, "");
    CHECK_INTEGER_FORMATS(signed char, unsigned char, "hh");
    CHECK_INTEGER_FORMATS(short, unsigned short, "h");
    CHECK_INTEGER_FORMATS(long, unsigned long, "l");
    CHECK_INTEGER_FORMATS(long long, unsigned long long, "ll");
    CHECK_INTEGER_FORMATS(std::intmax_t, std::uintmax_t, "j");
    CHECK_INTEGER_FORMATS(signed_size, std::size_t, "z");
    CHECK_INTEGER_FORMATS(std::ptrdiff_t, unsigned_difference, "t");

    CHECK_VALID_FORMAT("%hhd %hd", 257, 65537); // Conversion follows promotion.
    CHECK_VALID_FORMAT("%hhu %hu", 257u, 65537u);
    CHECK_VALID_FORMAT("%#08.4b %#*.*llB", 5u, 12, 8, 5ull);
    CHECK_INVALID_FORMAT("%d", 1L);
    CHECK_INVALID_FORMAT("%i", 1LL);
    CHECK_INVALID_FORMAT("%u", 1UL);
    CHECK_INVALID_FORMAT("%x", 1ULL);
    CHECK_INVALID_FORMAT("%ld", 1);
    CHECK_INVALID_FORMAT("%lld", 1L);
    CHECK_INVALID_FORMAT("%lu", 1ULL);
    CHECK_INVALID_FORMAT("%llu", 1UL);
    CHECK_INVALID_FORMAT("%hhd", 1L);
    CHECK_INVALID_FORMAT("%hu", 1UL);
    CHECK_INVALID_FORMAT("%u", -1);
    CHECK_INVALID_FORMAT("%b", -1);
    CHECK_INVALID_FORMAT("%ld", 1UL);
    CHECK_INVALID_FORMAT("%zu", signed_size(1));
    CHECK_INVALID_FORMAT("%td", unsigned_difference(1));
    CHECK_INVALID_FORMAT("%d", 1.0);
}

enum signedEnum : int { signedEnumValue = 1 };
enum unsignedEnum : unsigned int { unsignedEnumValue = 1 };
enum longEnum : long { longEnumValue = 1 };
enum unsignedLongEnum : unsigned long { unsignedLongEnumValue = 1 };
enum class scopedEnum : int { value = 1 };
enum overloadedEnum : long { overloadedEnumValue = 1 };
int operator+(overloadedEnum) { return 1; }

struct integerLike {
    operator int() const { return 1; }
};

TEST_CASE("enum promotions and class arguments") {
    CHECK_VALID_FORMAT("%d %u %ld %lx", signedEnumValue, unsignedEnumValue,
                       longEnumValue, unsignedLongEnumValue);
    CHECK_VALID_FORMAT("%*d", signedEnumValue, 1);
    CHECK_VALID_FORMAT("%ld", overloadedEnumValue);
    CHECK_INVALID_FORMAT("%d", overloadedEnumValue);
    CHECK_INVALID_FORMAT("%d", longEnumValue);
    CHECK_INVALID_FORMAT("%u", unsignedLongEnumValue);
    CHECK_INVALID_FORMAT("%d", scopedEnum::value);
    CHECK_INVALID_FORMAT("%u", scopedEnum::value);
    CHECK_INVALID_FORMAT("%*d", scopedEnum::value, 1);
    CHECK_INVALID_FORMAT("%d", integerLike{});
    CHECK_INVALID_FORMAT("%*d", integerLike{}, 1);
    CHECK_INVALID_FORMAT("%s", std::string("text"));
}

TEST_CASE_TEMPLATE("floating-point lengths:", T, float, double) {
    T value = 1.5;
    CHECK_VALID_FORMAT("%lf %lF %le %lE %la %lA %lg %lG",
                       value, value, value, value, value, value, value, value);
    CHECK_INVALID_FORMAT("%Lf", value);
}

TEST_CASE("long double needs the capital L modifier") {
    CHECK_VALID_FORMAT("%Lf %LF %Le %LE %La %LA %Lg %LG",
                       1.L, 1.L, 1.L, 1.L, 1.L, 1.L, 1.L, 1.L);
    CHECK_VALID_FORMAT("%+#*.*Lg", 10, 4, 1.L);
    CHECK_INVALID_FORMAT("%f", 1.L);
    CHECK_INVALID_FORMAT("%lf", 1.L);
    CHECK_INVALID_FORMAT("%Lf", 1.0);
    CHECK_INVALID_FORMAT("%f", 1);
}

TEST_CASE("narrow and wide characters and strings") {
    char text[] = "text";
    wchar_t wideText[] = L"wide";
    char* const constPointer = text;
    CHECK_VALID_FORMAT("%c %c", 'x', 65);
    CHECK_VALID_FORMAT("%lc %lc", L'x', std::wint_t(L'y'));
    CHECK_VALID_FORMAT("%s %s %s", text, "literal", constPointer);
    CHECK_VALID_FORMAT("%ls %ls", wideText, L"literal");
    CHECK_VALID_FORMAT("%-*.*ls", 8, 3, L"wide");
    CHECK_INVALID_FORMAT("%c", 65L);
    CHECK_INVALID_FORMAT("%c", "x");
    CHECK_INVALID_FORMAT("%lc", L"x");
    CHECK_INVALID_FORMAT("%s", L"wide");
    CHECK_INVALID_FORMAT("%ls", "narrow");
    CHECK_INVALID_FORMAT("%s", static_cast<volatile char*>(nullptr));
    CHECK_INVALID_FORMAT("%ls", static_cast<volatile wchar_t*>(nullptr));
    CHECK_INVALID_FORMAT("%s", static_cast<void*>(nullptr));
    CHECK_INVALID_FORMAT("%s", nullptr);
}

TEST_CASE("n writes through the exact signed pointer type") {
    signed char hh = 0;
    short h = 0;
    int n = 0;
    long l = 0;
    long long ll = 0;
    std::intmax_t j = 0;
    std::make_signed<std::size_t>::type z = 0;
    std::ptrdiff_t t = 0;
    CHECK_VALID_FORMAT("%hhn %hn %n %ln %lln %jn %zn %tn", &hh, &h, &n, &l, &ll, &j, &z, &t);
    CHECK_INVALID_FORMAT("%n", &h);
    CHECK_INVALID_FORMAT("%hn", &n);
    CHECK_INVALID_FORMAT("%hhn", static_cast<char*>(nullptr));
    CHECK_INVALID_FORMAT("%ln", &ll);
    CHECK_INVALID_FORMAT("%lln", &l);
    CHECK_INVALID_FORMAT("%zn", static_cast<std::size_t*>(nullptr));
    CHECK_INVALID_FORMAT("%n", static_cast<unsigned int*>(nullptr));
    CHECK_INVALID_FORMAT("%n", static_cast<const int*>(nullptr));
    CHECK_INVALID_FORMAT("%n", static_cast<volatile int*>(nullptr));
    CHECK_INVALID_FORMAT("%n", static_cast<void*>(nullptr));
    CHECK_INVALID_FORMAT("%n", nullptr);
}

TEST_CASE("p requires an explicit void pointer") {
    int value = 0;
    CHECK_VALID_FORMAT("%p", static_cast<void*>(&value));
    CHECK_VALID_FORMAT("%p", nullptr);
    CHECK_INVALID_FORMAT("%p", &value);
    CHECK_INVALID_FORMAT("%p", static_cast<const void*>(nullptr));
    CHECK_INVALID_FORMAT("%p", &std::puts);
    CHECK_INVALID_FORMAT("%p", 0);
}

TEST_CASE("pointer output includes the default nullptr conversion") {
    int value = 0;
    char expected[128] = {};
    char actual[128] = {};
    const int size = std::snprintf(expected, sizeof(expected), "%p|%p",
        static_cast<void*>(&value), static_cast<void*>(nullptr));
    CHECK(tssnprintf(actual, sizeof(actual), "%p|%p", static_cast<void*>(&value), nullptr) == size);
    CHECK(std::string(actual) == expected);
}

TEST_CASE("all n length modifiers write character counts") {
    signed char hh = 0;
    short h = 0;
    int n = 0;
    long l = 0;
    long long ll = 0;
    std::intmax_t j = 0;
    std::make_signed<std::size_t>::type z = 0;
    std::ptrdiff_t t = 0;
    char buffer[16] = {};
#ifdef _MSC_VER
    const int previous = _set_printf_count_output(1);
#endif
    const int result = tssnprintf(buffer, sizeof(buffer),
        "a%hhnb%hnc%nd%lne%llnf%jng%znh%tn", &hh, &h, &n, &l, &ll, &j, &z, &t);
#ifdef _MSC_VER
    _set_printf_count_output(previous);
#endif
    CHECK(result == 8);
    CHECK(std::string(buffer) == "abcdefgh");
    CHECK(hh == 1);
    CHECK(h == 2);
    CHECK(n == 3);
    CHECK(l == 4);
    CHECK(ll == 5);
    CHECK(j == 6);
    CHECK(z == 7);
    CHECK(t == 8);
}

TEST_CASE("malformed and undefined conversion combinations are rejected") {
    // Check incomplete prefixes at compile time so no parser path may read past '\0'.
    constexpr const char* incomplete[] = {
        "%", "%+", "%12", "%.", "%*", "%.*", "%*.*", "%h", "%hh", "%l", "%ll",
        "%j", "%z", "%t", "%L", "%08.3ll", "%#-+ 0*.*"
    };
    constexpr const char* invalid[] = {
        "%q", "%D", "%I64d", "%2$d", "%*2$d", "%..d", "%**d", "%.*3d", "%3-d",
        "%hhhhd", "%llld", "%hld", "%Ld", "%Lu", "%llf", "%hf", "%jf", "%zf", "%tf",
        "%hc", "%llc", "%Lc", "%hs", "%lls", "%Ls", "%lp", "%Ln",
        "%#d", "%#i", "%#u", "%#s", "%#c", "%#p", "%0s", "%0c", "%0p",
        "%.c", "%.2c", "%.*c", "%.p", "%.*p", "%+n", "% n", "%#n", "%0n",
        "%-n", "%1n", "%*n", "%.n", "%.*n", "%+%", "%1%", "%*%", "%.%", "%l%"
    };
    constexpr bool truncatedAreInvalid =
        allFormatsInvalid<tsprintf_impl::formatParser<int, int, int>>(incomplete);
    constexpr bool combinationsAreInvalid =
        allFormatsInvalid<tsprintf_impl::formatParser<int, int, int>>(invalid) &&
        allFormatsInvalid<tsprintf_impl::formatParser<double>>(invalid) &&
        allFormatsInvalid<tsprintf_impl::formatParser<char*>>(invalid) &&
        allFormatsInvalid<tsprintf_impl::formatParser<void*>>(invalid) &&
        allFormatsInvalid<tsprintf_impl::formatParser<int*>>(invalid) &&
        allFormatsInvalid<tsprintf_impl::formatParser<>>(invalid);
    CHECK(truncatedAreInvalid);
    CHECK(combinationsAreInvalid);
    for(auto fmt : invalid) {
        INFO(fmt);
        // These pointer types would otherwise satisfy c/s/p/n conversions.
        CHECK_FALSE(tsprintf_impl::formatParser<int, int, int>::checkFormat(fmt));
        CHECK_FALSE(tsprintf_impl::formatParser<double>::checkFormat(fmt));
        CHECK_FALSE(tsprintf_impl::formatParser<char*>::checkFormat(fmt));
        CHECK_FALSE(tsprintf_impl::formatParser<void*>::checkFormat(fmt));
        CHECK_FALSE(tsprintf_impl::formatParser<int*>::checkFormat(fmt));
        CHECK_FALSE(tsprintf_impl::formatParser<>::checkFormat(fmt));
    }
    CHECK_INVALID_FORMAT("%d trailing %", 1);
    CHECK_INVALID_FORMAT("%d%% %s", 1);
    CHECK_INVALID_FORMAT("%d%*.*f", 1, 8, 2);
    CHECK_INVALID_FORMAT("%%%%%", 1);
}

TEST_CASE("all output wrappers preserve printf formatting and return values") {
    const char* fmt = "%+#08x|%*.*f|%-6.3s|%hhd|%ld|%lld|%zu|%td|%lc|%.3ls|%%";
    char expected[256] = {};
    char actual[256] = {};
    int size = std::snprintf(expected, sizeof(expected), fmt,
        42u, -10, 2, 1.25, "hello", 257, -42L, 42LL, std::size_t(7),
        std::ptrdiff_t(-3), std::wint_t(L'W'), L"wide");
    REQUIRE(size > 0);
    int result = tssnprintf(actual, sizeof(actual),
        "%+#08x|%*.*f|%-6.3s|%hhd|%ld|%lld|%zu|%td|%lc|%.3ls|%%",
        42u, -10, 2, 1.25, "hello", 257, -42L, 42LL, std::size_t(7),
        std::ptrdiff_t(-3), std::wint_t(L'W'), L"wide");
    CHECK(result == size);
    CHECK(std::string(actual) == expected);
    result = tssprintf(actual, "%#08x|%.*Lf|%jd", 42u, 3, 1.25L, std::intmax_t(-7));
    size = std::sprintf(expected, "%#08x|%.*Lf|%jd", 42u, 3, 1.25L, std::intmax_t(-7));
    CHECK(result == size);
    CHECK(std::string(actual) == expected);

    std::FILE* file = std::tmpfile();
    REQUIRE(file != nullptr);
    result = tsfprintf(file, "%*.*f|%ls|%%", 8, 2, 1.25, L"wide");
    size = std::snprintf(expected, sizeof(expected), "%*.*f|%ls|%%", 8, 2, 1.25, L"wide");
    CHECK(result == size);
    std::rewind(file);
    std::memset(actual, 0, sizeof(actual));
    CHECK(std::fread(actual, 1, sizeof(actual)-1, file) == static_cast<std::size_t>(size));
    CHECK(std::string(actual) == expected);
    CHECK(std::fclose(file) == 0);

    CHECK(tsprintf("%s%*.*f%%\n", "checked: ", 6, 2, 1.25) == 17);
}

TEST_CASE("snprintf truncation and dynamic arguments retain their runtime semantics") {
    char actual[5] = {};
    CHECK(tssnprintf(actual, sizeof(actual), "%*.*f", 8, 2, 1.25) == 8);
    CHECK(std::string(actual) == "    ");
    CHECK(tssnprintf(nullptr, 0, "%*.*f", 8, 2, 1.25) == 8);
    int width = 6;
    int precision = 2;
    double value = 1.25;
    char full[32] = {};
    CHECK(tssnprintf(full, sizeof(full), "%*.*f", width++, precision++, value++) == 6);
    CHECK(width == 7);
    CHECK(precision == 3);
    CHECK(value == 2.25);
    CHECK(std::string(full) == "  1.25");
    CHECK(tssnprintf(full, sizeof(full), "%*.*f", -6, -1, 1.25) == 8);
    CHECK(std::string(full) == "1.250000");
}
