
#include "utils.h"

std::string getCurrentTimestamp() {
    namespace pt = boost::posix_time;
    // 取当前本地时间（精确到秒）
    pt::ptime now = pt::second_clock::local_time();
    // 用 time_facet 指定格式
    std::ostringstream oss;
    static std::locale loc(std::locale::classic(),
        new pt::time_facet("%Y-%m-%d %H:%M:%S"));
    oss.imbue(loc);
    oss << now;
    return oss.str();
}