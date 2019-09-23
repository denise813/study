#ifndef TRACE_H
#define TRACE_H
#include <iostream>
#include "boost/log/trivial.hpp"
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>


using std::cout;
using std::endl;

using boost::log;
using boost::trivial;


class Trace
{
public:
    Trace();
    virtual ~Trace();
};

#if 0
    ctime(&t) << __FILE__ << __func__ << __LINE__ << level
#define TRACE(level) BOOST_LOG_TRIVIAL(level)
#define TRACE(level) BOOST_LOG_TRIVIAL(level)

#endif


#endif
