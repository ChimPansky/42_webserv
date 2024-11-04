#include <sstream>
#include <ctime>
#include <iomanip>

std::string GetFormatedTime(std::time_t raw_time) {
    if (0 == raw_time) {
        std::time(&raw_time);  // get currnet time
    }

    // Convert to GMT/UTC time
    struct std::tm *time_info = std::gmtime(&raw_time);

    // Format the time as "Day, DD Mon YYYY HH:MM:SS GMT"
    const char *dayNames[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    const char *monthNames[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                                "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    std::ostringstream oss;
    oss << dayNames[time_info->tm_wday] << ", "
        << std::setfill('0') << std::setw(2) << time_info->tm_mday << " "
        << monthNames[time_info->tm_mon] << " "
        << (1900 + time_info->tm_year) << " "
        << std::setw(2) << std::setfill('0') << time_info->tm_hour << ":"
        << std::setw(2) << std::setfill('0') << time_info->tm_min << ":"
        << std::setw(2) << std::setfill('0') << time_info->tm_sec
        << " GMT";

    return oss.str();
}
