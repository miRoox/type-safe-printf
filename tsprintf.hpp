/**
 ** Copyright (c) 2017-2020 miRoox.
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
#include <cstdio>

namespace tsprintf_impl {

// alias
using constStr = const char *;
using std::size_t;

// if return 0, it means failed
// else the return value means the offset of this format
template<typename T>
constexpr size_t checkOneFormat(constStr fmt)
{
    size_t offset = 0;
    if(fmt[0] == '%') {
        //TODO: add prefix
        if(fmt[offset+1] == 'c' ||
           fmt[offset+1] == 'd' ||
           fmt[offset+1] == 'i') { //int
            if(std::is_integral<T>::value || std::is_enum<T>::value) {
                return ++offset;
            }
            else {
                return 0;
            }
        }
        else if (fmt[offset+1] == 's') { //char*
            if(std::is_pointer<T>::value &&
                    std::is_same<typename std::remove_cv<typename std::remove_pointer<T>::type>::type, char>::value) {
                return ++offset;
            }
            else {
                return 0;
            }
        }
        else if (fmt[offset+1] == 'o' ||
                 fmt[offset+1] == 'x' || fmt[offset+1] == 'X' ||
                 fmt[offset+1] == 'u') { // unsigned int
            if((std::is_integral<T>::value && std::is_unsigned<T>::value) || std::is_enum<T>::value) {
                return ++offset;
            }
            else {
                return 0;
            }
        }
        else if (fmt[offset+1] == 'f' || fmt[offset+1] == 'F' ||
                 fmt[offset+1] == 'e' || fmt[offset+1] == 'E' ||
                 fmt[offset+1] == 'a' || fmt[offset+1] == 'A' ||
                 fmt[offset+1] == 'g' || fmt[offset+1] == 'G') { //double
            if(std::is_floating_point<T>::value) {
                return ++offset;
            }
            else {
                return 0;
            }
        }
        else if (fmt[offset+1] == 'n') { //int*
            if(std::is_pointer<T>::value &&
                    std::is_integral<typename std::remove_pointer<T>::type>::value) {
                return ++offset;
            }
            else {
                return 0;
            }
        }
        else if (fmt[offset+1] == 'p') { //void*
            if(std::is_pointer<T>::value) {
                return ++offset;
            }
            else {
                return 0;
            }
        }
        else { // unknown
            return 0;
        }
    }
    return offset;
}

// some forward declarations
constexpr bool checkNoFormat(constStr fmt);
template<typename Arg> constexpr bool checkFormat(constStr fmt);
template<typename Arg,typename Arg2,typename... Args> constexpr bool checkFormat(constStr fmt);


template<typename Arg,typename Arg2,typename... Args>
constexpr bool checkFormat(constStr fmt)
{
    for(size_t i = 0; fmt[i]; ++i) {
        if(fmt[i] == '%') {
            if(fmt[i+1] != '%') {
                size_t offset = checkOneFormat<Arg>(fmt+i);
                if(offset) {
                    return checkFormat<Arg2,Args...>(fmt+i+offset);
                }
                else {
                    return false;//invalid format
                }
            }
            else { //skip "%%"
                ++i;
            }
        }
    }
    return false;// need more arguments
}

template<typename Arg>
constexpr bool checkFormat(constStr fmt)
{
    for(size_t i = 0; fmt[i]; ++i) {
        if(fmt[i] == '%') {
            if(fmt[i+1] != '%') {
                size_t offset = checkOneFormat<Arg>(fmt+i);
                if(offset) {
                    return checkNoFormat(fmt+i+offset);
                }
                else {
                    return false;//invalid format
                }
            }
            else { //skip "%%"
                ++i;
            }
        }
    }
    return false;// need more argument
}

// check no format
constexpr bool checkNoFormat(constStr fmt)
{
    for(size_t i = 0; fmt[i]; ++i) {
        if(fmt[i] == '%') {
            if(fmt[i+1] != '%') {
                return false;//too many argument
            }
            else { //skip "%%"
                ++i;
            }
        }
    }
    return true;
}


// this class is just for template parameters
template<typename ...T>
struct formatParser
{
    static constexpr bool checkFormat(constStr fmt)
    {
        return tsprintf_impl::checkFormat<T...>(fmt);
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

// this function is use to generate a formatParser with decltype()
// so we don't need a definition for this
template<typename ...T>
formatParser<T...> getParser(constStr fmt, T ...args);

}// end namespace tsprintf_impl

#define _tsprintf_get_fmt(fmt,...) fmt

#define tsprintf(...) [&]()->int{ \
    using namespace tsprintf_impl;         \
    static_assert(decltype(getParser(__VA_ARGS__))::checkFormat(_tsprintf_get_fmt(__VA_ARGS__)), \
    "tprintf(" #__VA_ARGS__ ") has wrong foramt");        \
    return std::printf(__VA_ARGS__);}()

#endif // TSPRINTF_HPP
