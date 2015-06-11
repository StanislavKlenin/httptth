#include "logger.hpp"

#include <stdarg.h>

namespace httptth
{

logger::logger(std::unique_ptr<transport> t)
{
    transports.push_back(std::move(t));
}

void logger::log(enum level level, const char *data, size_t length)
{
    for (auto &t : transports) {
        if (t->supports(level)) {
            t->log(level, data, length);
        }
    }
}

void logger::add_transport(std::unique_ptr<transport> t)
{
    transports.push_back(std::move(t));
}

const char * logger::level_names[] = {
    "EMERG",
    "ALERT",
    "CRIT",
    "ERR",
    "WARNING",
    "NOTICE",
    "INFO",
    "DEBUG"
};

} // namespace httptth
