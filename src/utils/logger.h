#ifndef WS_UTILS_LOGGER_H
#define WS_UTILS_LOGGER_H

#include <fstream>
#include <ostream>

#define LOGGER_TIME_FORMAT "%Y%m%d %H:%M:%S"
#define LOGGER_TIME_FORMAT_MAX_LEN 25

enum Severity {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
    FATAL = 4
};

namespace utils {

class Logger {
  private:                             // not copyable
    Logger(const Logger&);             // = delete;
    Logger& operator=(const Logger&);  // = delete;

  private:
    Logger(char* log_path = NULL);  // singleton initialisation
    class NullStream : public std::ostream {
      public:
        NullStream() : std::ostream(&nullBuffer) {}
      private:
        class NullBuffer : public std::streambuf {
        public:
            int overflow(int c) {
                return c;
            }
        };
        NullBuffer nullBuffer;
    };
    class LogWrapper {
      public:
        LogWrapper(std::ostream& os, Severity sev);
        ~LogWrapper() throw(std::runtime_error);
        template <typename T>
        LogWrapper& operator<<(const T& rhs)
        {
            os_ << rhs;
            return *this;
        }

      private:
        std::ostream& os_;
        Severity sev_;
    };

  public:
    static Logger& get();
    static char* dump_time();
    LogWrapper log(enum Severity sev);
    void set_severity_threshold(Severity);

  private:
    std::ofstream fs_;
    std::ostream* os_;
    std::ostream* es_;
    NullStream    null_stream_;

    Severity severity_threshold_;

    static char format_buf_[LOGGER_TIME_FORMAT_MAX_LEN];
};

}  // namespace utils

#define LOG(severity)                                                                       \
    utils::Logger::get().log(severity) << #severity[0] << utils::Logger::dump_time() << " " \
                                       << __FILE__ << ":" << __LINE__ << "] "

#define LOG_IF(severity, condition) if (condition) LOG(severity)

#endif  // WS_UTILS_LOGGER_H
