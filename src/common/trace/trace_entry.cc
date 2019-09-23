#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>


namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace keywords = boost::log::keywords;
 
 
TraceEntry::TraceEntry()
{
}

TraceEntry::~TraceEntry()
{
}


void TraceEntry::init()
{
    auto file_sink = logging::add_file_log (
                    keywords::file_name = "sample_%N.log",                                        /*< file name pattern >*/
                    keywords::rotation_size = 10 * 1024 * 1024,                                   /*< rotate files every 10 MiB... >*/
                    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0), /*< ...or at midnight >*/
                    keywords::format = "[%TimeStamp%]: %Message%"                                 /*< log record format >*/
    );
 
    file_sink->locked_backend()->set_file_collector(sinks::file::make_collector (
                    keywords::target = "logs",        //folder name.
                    keywords::max_size = 50 * 1024 * 1024,    //The maximum amount of space of the folder.
                    keywords::min_free_space = 100 * 1024 * 1024  //Reserved disk space minimum.
    ));
 
    file_sink->locked_backend()->scan_for_files();
    
    logging::core::get()->set_filter
    (
        logging::trivial::severity >= logging::trivial::info
    );
}

Trace::Trace()
{
}

Trace::~Trace()
{

}

