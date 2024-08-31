#include "logger.h"

#include <iostream>
#include <fstream>
#include <ctime>
#include <stdexcept>


using namespace utils;

Logger::Logger(char* log_path)
{
    if ((log_path && *log_path != '\0')) {
        fs_.open(log_path);
    }
    os_ = (fs_.is_open() ? &fs_ : &std::cout);
    es_ = (fs_.is_open() ? &fs_ : &std::cerr);
}


Logger::LogWrapper::LogWrapper(std::ostream& os, Severity sev)
    : os_(os), sev_(sev)
{}


Logger::LogWrapper::~LogWrapper() {
    os_ << std::endl;
    if (sev_ == FATAL) {
        throw std::runtime_error("fatal error");
    }
}

char* Logger::dump_time() {
    // add usec?
    time_t rawtime = time(NULL);
    struct tm* timeinfo;
    timeinfo = localtime(&rawtime);
    strftime(format_buf_, LOGGER_TIME_FORMAT_MAX_LEN, LOGGER_TIME_FORMAT, timeinfo);
    return format_buf_;
}

Logger::LogWrapper Logger::log(enum Severity sev) {
    std::ostream& os = (sev < WARNING ? *os_ : *es_);
    // possible color:
    // if (dynamic_cast<std::ofstream*>(&os) == NULL) {

    // }
    return LogWrapper(os, sev);
}

Logger& Logger::get() {
    if(!logger) {
        logger = new Logger;
    }
    return *logger;
}

Logger* Logger::logger = NULL;
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
