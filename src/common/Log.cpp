#include "Log.h"

void Log::init(bool logToConsole)
{
    boost::log::add_common_attributes();
    if (logToConsole)
    {
        logging::add_console_log
        (
            std::cout,
            keywords::auto_flush = true,
            keywords::format = "[%TimeStamp%] %Message%"
        );
        BOOST_LOG_TRIVIAL(debug) << "Logging started.";
    }
    else
    {
        logging::add_file_log
        (
            keywords::file_name = "/var/log/streamwriter/streamwriter_%N.log",
            keywords::rotation_size = 10 * 1024 * 1024,
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
            keywords::format = "[%TimeStamp%] %Message%"
        );
    }
}
