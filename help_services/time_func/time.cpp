
#include <iostream>
#include <ctime>
#include <iomanip>
#include <sstream>

std::string get_time() {
    std::time_t now = std::time(nullptr);        
    std::tm localTime = *std::localtime(&now);   
    
    std::ostringstream oss;

    oss << std::setfill('0') << std::setw(2) << localTime.tm_hour << ":"
        << std::setfill('0') << std::setw(2) << localTime.tm_min << ":"
        << std::setfill('0') << std::setw(2) << localTime.tm_sec;
    
    return oss.str();
}