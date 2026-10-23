/**
 ** Copyright (c) 2017-2022 Yong-an Lu.
 **
 ** Permission is hereby granted, free of charge, to any person obtaining a copy
 ** of this software and associated documentation files (the "Software"), to deal
 ** in the Software without restriction, including without limitation the rights
 ** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 ** copies of the Software, and to permit persons to whom the Software is
 ** furnished to do so, subject to the following conditions:
 **
 ** The above copyright notice and this permission notice shall be included in all
 ** copies or substantial portions of the Software.
 **
 ** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 ** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 ** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 ** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 ** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM
 ** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 ** SOFTWARE.
 **/

#ifndef TSPRINTF_HPP
#define TSPRINTF_HPP

#include <type_traits>
#include <cstddef>
#include <cstdio>
#include <cstdint>
#include <cwchar>
#include <utility>

namespace tsprintf_impl {

// alias
using constStr = const char *;
using ::std::size_t;

enum class lengthModifier { none, hh, h, l, ll, j, z, t, L };

enum formatFlag : unsigned {
    left = 1, sign = 2, space = 4, alternate = 8, zero = 16
};

struct formatSpec {
    size_t offset = 0; // Index of the conversion character; zero means invalid.
    unsigned flags = 0;
    bool width = false;
    bool precision = false;
    bool widthArgument = false;
    bool precisionArgument = false;
    lengthModifier length = lengthModifier::none;
    char conversion = '\0';
};

constexpr bool isDigit(char ch) { return ch >= '0' && ch <= '9'; }

constexpr bool isInteger(char ch)
{
    return ch == 'd' || ch == 'i' || ch == 'o' || ch == 'u' ||
           ch == 'x' || ch == 'X' || ch == 'b' || ch == 'B';
}

constexpr bool isFloating(char ch)
{
    return ch == 'f' || ch == 'F' || ch == 'e' || ch == 'E' ||
           ch == 'a' || ch == 'A' || ch == 'g' || ch == 'G';
}

constexpr unsigned flagValue(char ch)
{
    return ch == '-' ? left : ch == '+' ? sign : ch == ' ' ? space :
           ch == '#' ? alternate : ch == '0' ? zero : 0;
}

constexpr formatSpec parseFormat(constStr fmt)
{
    formatSpec spec;
    if(fmt[0] != '%') {
        return spec;
    }

    size_t i = 1;
    while(flagValue(fmt[i])) {
        spec.flags |= flagValue(fmt[i++]);
    }
    if(fmt[i] == '*') {
        spec.width = spec.widthArgument = true;
        ++i;
    }
    else {
        while(isDigit(fmt[i])) {
            spec.width = true;
            ++i;
        }
    }
    if(fmt[i] == '.') {
        spec.precision = true;
        ++i;
        if(fmt[i] == '*') {
            spec.precisionArgument = true;
            ++i;
        }
        else {
            while(isDigit(fmt[i])) {
                ++i;
            }
        }
    }

    switch(fmt[i]) {
    case 'h':
        spec.length = lengthModifier::h;
        if(fmt[++i] == 'h') {
            spec.length = lengthModifier::hh;
            ++i;
        }
        break;
    case 'l':
        spec.length = lengthModifier::l;
        if(fmt[++i] == 'l') {
            spec.length = lengthModifier::ll;
            ++i;
        }
        break;
    case 'j': spec.length = lengthModifier::j; ++i; break;
    case 'z': spec.length = lengthModifier::z; ++i; break;
    case 't': spec.length = lengthModifier::t; ++i; break;
    case 'L': spec.length = lengthModifier::L; ++i; break;
    }
    spec.conversion = fmt[i];

    const bool integer = isInteger(spec.conversion);
    const bool floating = isFloating(spec.conversion);
    if(integer || spec.conversion == 'n') {
        if(spec.length == lengthModifier::L) {
            return spec;
        }
    }
    else if(floating || spec.conversion == 'c' || spec.conversion == 's') {
        if(spec.length != lengthModifier::none && spec.length != lengthModifier::l &&
           !(floating && spec.length == lengthModifier::L)) {
            return spec;
        }
    }
    else if(spec.conversion == 'p' || spec.conversion == '%') {
        if(spec.length != lengthModifier::none) {
            return spec;
        }
    }
    else {
        return spec;
    }

    if(spec.conversion == 'n' || spec.conversion == '%') {
        if(spec.flags || spec.width || spec.precision) {
            return spec;
        }
    }
    if((spec.flags & alternate) && !floating && spec.conversion != 'o' &&
       spec.conversion != 'x' && spec.conversion != 'X' &&
       spec.conversion != 'b' && spec.conversion != 'B') {
        return spec;
    }
    if((spec.flags & zero) && !integer && !floating) {
        return spec;
    }
    if(spec.precision && !integer && !floating && spec.conversion != 's') {
        return spec;
    }

    spec.offset = i;
    return spec;
}

template<typename...> using void_t = void;

// Combining with int performs integral promotions, including those of unscoped enums.
// Unlike unary +, the conditional operator cannot be overloaded by an enum.
template<typename T, typename = void>
struct promotedType { using type = void; };

template<typename T>
struct promotedType<T, void_t<decltype(false ? ::std::declval<T>() : 0)>> {
    using type = typename ::std::conditional<
        ::std::is_integral<T>::value || ::std::is_enum<T>::value,
        decltype(false ? ::std::declval<T>() : 0), void>::type;
};

template<typename T, bool = ::std::is_enum<T>::value>
struct integerSource { using type = T; };

template<typename T>
struct integerSource<T, true> { using type = typename ::std::underlying_type<T>::type; };

template<typename T, typename Expected>
constexpr bool matchesInteger()
{
    using promoted = typename promotedType<T>::type;
    using expected = typename promotedType<Expected>::type;
    if(::std::is_unsigned<Expected>::value) {
        // Preserve the unsigned-only policy, while allowing promotions of small unsigneds.
        return ::std::is_unsigned<typename integerSource<T>::type>::value &&
            (::std::is_same<promoted, expected>::value ||
             ((::std::is_same<Expected, unsigned char>::value ||
               ::std::is_same<Expected, unsigned short>::value ||
               ::std::is_same<Expected, unsigned int>::value) &&
              (::std::is_same<promoted, int>::value ||
               ::std::is_same<promoted, unsigned int>::value)));
    }
    return ::std::is_same<promoted, expected>::value;
}

template<typename Arg>
constexpr bool matchesValue(formatSpec spec)
{
    using T = typename ::std::decay<Arg>::type;
    using signed_size = typename ::std::make_signed<size_t>::type;
    using unsigned_difference = typename ::std::make_unsigned<::std::ptrdiff_t>::type;
    if(isInteger(spec.conversion)) {
        const bool signedConversion = spec.conversion == 'd' || spec.conversion == 'i';
        switch(spec.length) {
        case lengthModifier::none:
            return signedConversion ? matchesInteger<T, int>() : matchesInteger<T, unsigned int>();
        case lengthModifier::hh:
            return signedConversion ? matchesInteger<T, signed char>() : matchesInteger<T, unsigned char>();
        case lengthModifier::h:
            return signedConversion ? matchesInteger<T, short>() : matchesInteger<T, unsigned short>();
        case lengthModifier::l:
            return signedConversion ? matchesInteger<T, long>() : matchesInteger<T, unsigned long>();
        case lengthModifier::ll:
            return signedConversion ? matchesInteger<T, long long>() : matchesInteger<T, unsigned long long>();
        case lengthModifier::j:
            return signedConversion ? matchesInteger<T, ::std::intmax_t>() : matchesInteger<T, ::std::uintmax_t>();
        case lengthModifier::z:
            return signedConversion ? matchesInteger<T, signed_size>() : matchesInteger<T, size_t>();
        case lengthModifier::t:
            return signedConversion ? matchesInteger<T, ::std::ptrdiff_t>() : matchesInteger<T, unsigned_difference>();
        default: return false;
        }
    }
    if(isFloating(spec.conversion)) {
        return spec.length == lengthModifier::L ? ::std::is_same<T, long double>::value :
            (::std::is_same<T, float>::value || ::std::is_same<T, double>::value);
    }
    if(spec.conversion == 'c') {
        return spec.length == lengthModifier::l ?
            (::std::is_same<typename promotedType<T>::type,
                           typename promotedType<::std::wint_t>::type>::value ||
             ::std::is_same<T, wchar_t>::value) :
            ::std::is_same<typename promotedType<T>::type, int>::value;
    }
    if(spec.conversion == 's') {
        return spec.length == lengthModifier::l ?
            (::std::is_same<T, wchar_t*>::value || ::std::is_same<T, const wchar_t*>::value) :
            (::std::is_same<T, char*>::value || ::std::is_same<T, const char*>::value);
    }
    if(spec.conversion == 'p') {
        // nullptr_t is converted to void* when passed through an ellipsis.
        return ::std::is_same<T, void*>::value || ::std::is_same<T, ::std::nullptr_t>::value;
    }
    if(spec.conversion == 'n') {
        switch(spec.length) {
        case lengthModifier::none: return ::std::is_same<T, int*>::value;
        case lengthModifier::hh: return ::std::is_same<T, signed char*>::value;
        case lengthModifier::h: return ::std::is_same<T, short*>::value;
        case lengthModifier::l: return ::std::is_same<T, long*>::value;
        case lengthModifier::ll: return ::std::is_same<T, long long*>::value;
        case lengthModifier::j: return ::std::is_same<T, ::std::intmax_t*>::value;
        case lengthModifier::z: return ::std::is_same<T, signed_size*>::value;
        case lengthModifier::t: return ::std::is_same<T, ::std::ptrdiff_t*>::value;
        default: return false;
        }
    }
    return false;
}

// Returns the index of the conversion character, or zero on failure.
template<typename T>
constexpr size_t checkOneFormat(constStr fmt)
{
    const auto spec = parseFormat(fmt);
    return spec.offset && !spec.widthArgument && !spec.precisionArgument &&
        matchesValue<T>(spec) ? spec.offset : 0;
}

// check no format
constexpr bool checkNoFormat(constStr fmt)
{
    for(size_t i = 0; fmt[i]; ++i) {
        if(fmt[i] == '%') {
            if(fmt[i+1] != '%') {
                return false; // A conversion remains without an argument.
            }
            else { //skip "%%"
                ++i;
            }
        }
    }
    return true;
}


template<typename... T> struct formatParser;
template<typename... T> struct argumentChecker;

// Width and precision consume arguments before the converted value.
template<typename Arg, typename... Rest>
struct argumentChecker<Arg, Rest...> {
    static constexpr bool check(formatSpec spec, constStr next)
    {
        if(spec.widthArgument) {
            spec.widthArgument = false;
            return ::std::is_same<typename promotedType<typename ::std::decay<Arg>::type>::type, int>::value &&
                argumentChecker<Rest...>::check(spec, next);
        }
        if(spec.precisionArgument) {
            spec.precisionArgument = false;
            return ::std::is_same<typename promotedType<typename ::std::decay<Arg>::type>::type, int>::value &&
                argumentChecker<Rest...>::check(spec, next);
        }
        return matchesValue<Arg>(spec) && formatParser<Rest...>::checkFormat(next);
    }
};

template<>
struct argumentChecker<> {
    static constexpr bool check(formatSpec, constStr) { return false; }
};

template<typename Arg, typename... Rest>
struct formatParser<Arg, Rest...> {
    static constexpr bool checkFormat(constStr fmt)
    {
        for(size_t i = 0; fmt[i]; ++i) {
            if(fmt[i] == '%') {
                const auto spec = parseFormat(fmt+i);
                if(!spec.offset) {
                    return false;
                }
                if(spec.conversion == '%') {
                    i += spec.offset;
                }
                else {
                    return argumentChecker<Arg, Rest...>::check(spec, fmt+i+spec.offset+1);
                }
            }
        }
        return false; // Arguments remain without a conversion.
    }
};

// specialization
template<>
struct formatParser<>
{
    static constexpr bool checkFormat(constStr fmt)
    {
        return tsprintf_impl::checkNoFormat(fmt);
    }
};

template<typename... Args>
constexpr bool checkFormat(constStr fmt)
{
    return formatParser<Args...>::checkFormat(fmt);
}

// this function is use to generate a formatParser with decltype()
// so we don't need a definition for this
template<typename ...T>
formatParser<T...> getParser(constStr fmt, T ...args);

}// end namespace tsprintf_impl

#define _Expand_MSVC(...) __VA_ARGS__
#define _tsprintf_get_fmt(fmt, ...) fmt
#define _tsprintf_check_fmt(...) (decltype(::tsprintf_impl::getParser(__VA_ARGS__))::checkFormat(_Expand_MSVC(_tsprintf_get_fmt(__VA_ARGS__))))

#define tsprintf(...) [&]()->int{ \
    static_assert(_tsprintf_check_fmt(__VA_ARGS__), \
    "tsprintf(" #__VA_ARGS__ ") has wrong format");        \
    return ::std::printf(__VA_ARGS__);}()

#define tsfprintf(stream, ...) [&]()->int{ \
    static_assert(_tsprintf_check_fmt(__VA_ARGS__), \
    "tsfprintf(" #stream ", " #__VA_ARGS__ ") has wrong format");        \
    return ::std::fprintf(stream, __VA_ARGS__);}()

#define tssprintf(buffer, ...) [&]()->int{ \
    static_assert(_tsprintf_check_fmt(__VA_ARGS__), \
    "tssprintf(" #buffer ", " #__VA_ARGS__ ") has wrong format");        \
    return ::std::sprintf(buffer, __VA_ARGS__);}()

#define tssnprintf(buffer, size, ...) [&]()->int{ \
    static_assert(_tsprintf_check_fmt(__VA_ARGS__), \
    "tssnprintf(" #buffer ", " #size ", " #__VA_ARGS__ ") has wrong format");        \
    return ::std::snprintf(buffer, size, __VA_ARGS__);}()

#endif // TSPRINTF_HPP
