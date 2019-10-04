#ifndef _LOGGER_GUARD_H
#define _LOGGER_GUARD_H
#include <mutex>
#define NO_LOGGER
namespace Logger {
extern std::mutex log_mutex;
}
#ifdef NO_LOGGER
#    define TRACE(x)
#else
/// This is not sophisticated, nor high performabnce but it works adequately for
/// a demo project
#    define TRACE(x)                                                                                                                                           \
        do {                                                                                                                                                   \
            std::lock_guard<std::mutex> guard(Logger::log_mutex);                                                                                              \
            std::cout << "T[" << __PRETTY_FUNCTION__ << "(" << __LINE__ << ")]::" << x << std::endl;                                                           \
        } while (0)

#    define Trace(x) TRACE(x)
#endif // NO_LOGGER
#endif
