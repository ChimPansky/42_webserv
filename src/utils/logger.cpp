#include "logger.h"

#include <unistd.h>

#include <ctime>
#include <fstream>
#include <iostream>
#include <stdexcept>

using namespace utils;

Logger::NullStream Logger::null_stream_;

Logger::Logger(char* log_path) : severity_threshold_(DEBUG)
{
    if ((log_path && *log_path != '\0')) {
        fs_.open(log_path);
    }
    os_ = (fs_.is_open() ? &fs_ : &std::cout);
    es_ = (fs_.is_open() ? &fs_ : &std::cerr);
}

Logger::LogWrapper::LogWrapper(std::ostream& os, Severity sev) : os_(os), sev_(sev)
{}

Logger::LogWrapper::~LogWrapper() throw(std::runtime_error)
{
    os_ << std::endl;
    if (sev_ == FATAL) {
        throw std::runtime_error("fatal error");
    }
}

char* Logger::dump_time()
{
    // add usec?
    time_t rawtime = time(NULL);
    struct tm* timeinfo;
    timeinfo = localtime(&rawtime);
    strftime(format_buf_, LOGGER_TIME_FORMAT_MAX_LEN, LOGGER_TIME_FORMAT, timeinfo);
    return format_buf_;
}

Logger::LogWrapper Logger::log(enum Severity sev)
{
    if (!std::cerr) {
        std::cout << std::cerr.bad() << std::cerr.fail() << std::cerr.eof() << std::endl;
        while (1) {
            // std::cout << " AAAAAAAAAA " << std::cerr.bad() << std::cerr.fail() << std::cerr.eof()
            // << std::endl;
        };
        std::cout << "FUCK" << std::endl;
        throw 1;
    }
    if (sev < severity_threshold_) {
        return LogWrapper(null_stream_, sev);
    }
    std::ostream& os = (sev < WARNING ? *os_ : *es_);
    // possible color:
    // if (dynamic_cast<std::ofstream*>(&os) == NULL) {

    // }
    return LogWrapper(os, sev);
}

Logger& Logger::get()
{
    static Logger logger;
    return logger;
}

void Logger::set_severity_threshold(Severity sev)
{
    severity_threshold_ = sev;
}

char Logger::format_buf_[LOGGER_TIME_FORMAT_MAX_LEN] = {};

// int main() {
//     // std::cout.operator<<(123);
//     // utils ::Logger ::get()->log(INFO) << 123;
//     LOG(DEBUG) << 123 << 321 << "asd";
//     LOG(INFO) << 123 << 321 << "asd";
//     LOG(WARNING) << 123 << 321 << "asd";
//     LOG(ERROR) << 123 << 321 << "asd";
//     LOG(FATAL) << 123 << 321 << "asd";
// }
