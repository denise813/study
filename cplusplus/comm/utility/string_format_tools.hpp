#ifndef _STRING_FORMAT_TOOLS_HPP_
#define _STRING_FORMAT_TOOLS_HPP_

#include <stdio.h>
#include <memory>


using namespace std;


#define string_format(format, ...) \
({  \
    std::string out; \
    snprintf(nullptr, 0, format, ##__VA_ARGS__); \
    out = _string_format(format, ##__VA_ARGS__);  \
    (out); \
})


template<typename ... Args>
static std::string _string_format(const std::string format, Args ... args)
{
    auto size_buf = snprintf(nullptr, 0, format.c_str(), args ...);
    size_buf = size_buf + 1;
    std::unique_ptr<char[]> string_buf(new char[size_buf]);
    snprintf(string_buf.get(), size_buf, format.c_str(), args ...);
    return std::string(string_buf.get(), string_buf.get() + size_buf - 1);
}


#endif
