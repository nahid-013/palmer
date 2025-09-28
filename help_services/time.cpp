#include <asio.hpp>
#include <iomanip>
#include <ctime>
#include "time.h"
#include <iostream>
#include <string>
std::string get_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}