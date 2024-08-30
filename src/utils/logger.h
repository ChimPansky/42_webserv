#include <iostream>
#include <fstream>
#include <ctime>
#include <stdexcept>

enum Severity
{
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

namespace utils {

class Logger {
  private:
    Logger(char* log_path = NULL)
    {
        if ((log_path && *log_path != '\0')) {
            fs_.open(log_path);
        }
        os_ = (fs_.is_open() ? &fs_ : &std::cout);
        es_ = (fs_.is_open() ? &fs_ : &std::cerr);
    }
  public:
    class LogWrapper {
      public:
        LogWrapper(std::ostream& os, Severity sev)
            : os_(os), sev_(sev)
        {}
        ~LogWrapper() {
            os_ << std::endl;
            if (sev_ == FATAL) {
                throw std::runtime_error("fatal error");
            }
        };
        public:
        template <typename T>
        LogWrapper& operator<<(const T& rhs) {os_ << rhs; return *this;}
      private:
        std::ostream& os_;
        Severity sev_;
    };

    static char* dump_time() {
        // add usec?
        time_t rawtime = time(NULL);
        struct tm* timeinfo;
        timeinfo = localtime(&rawtime);
        strftime(format_buf_, time_format_len_, "%Y%m%d %H:%M:%S", timeinfo);
        return format_buf_;
    }

    LogWrapper log(enum Severity sev) {
        std::ostream& os = (sev < WARNING ? *os_ : *es_);
        // possible color:
        // if (dynamic_cast<std::ofstream*>(&os) == NULL) {

        // }
        return LogWrapper(os, sev);
    }

    static Logger* get() {
        if(!logger) {
            logger = new Logger;
        }
        return logger;
    }
  private:
    std::ofstream fs_;
    std::ostream* os_;
    std::ostream* es_;

    static const int time_format_len_ = 25;
    static char format_buf_[time_format_len_];

    static Logger* logger;
};

Logger* Logger::logger = NULL;
char Logger::format_buf_[time_format_len_] = {};

}  // namespace utils

#define LOG(severity) utils::Logger::get()->log(severity) \
    << #severity[0] << utils::Logger::dump_time() << " " \
    << __FILE__ << ":" << __LINE__ << " ] "

/*
int main() {
    // std::cout.operator<<(123);
    // utils ::Logger ::get()->log(INFO) << 123;
    LOG(INFO) << 123 << 321 << "asd";
    LOG(DEBUG) << 123 << 321 << "asd";
    LOG(ERROR) << 123 << 321 << "asd";
    LOG(FATAL) << 123 << 321 << "asd";
}
*/
