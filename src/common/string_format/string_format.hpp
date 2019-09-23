#ifndef _YS_STRING_FORMAT_H
#define _YS_STRING_FORMAT_H
#include <boost/format.hpp>


using namespace std;


namespace yuanshuo
{
namespace tools
{


template<typename ... Args>
static std::string string_format(const std::string &format, Args ... args)
{
    boost::format b_format(format);
    std::initializer_list<char> {
        (static_cast<void>(b_format % args), char{}) ...};
    return boost::str(b_format);
}


};
};


#endif
