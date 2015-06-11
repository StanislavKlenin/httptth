#ifndef __LOGGER_FILE_H__
#define __LOGGER_FILE_H__

#include "common.hpp"

#include <string>

namespace httptth
{

class file_transport : public priority_limited_transport
{
public:
    file_transport(const char *filename, logger::level limit = logger::DEBUG);
    file_transport(FILE *stream, logger::level limit = logger::DEBUG) :
        priority_limited_transport(limit),
        file(stream),
        owner(false)
    {}
    virtual ~file_transport();
    
    void log(logger::level level, const char *data, size_t length) override;
    void format(logger::level level, const char *format, ...) override;
    
private:
    FILE        *file;
    bool         owner;
    std::string  fmt;
};

class stderr_transport : public file_transport
{
public:
    stderr_transport(logger::level limit = logger::DEBUG) :
        file_transport(stderr, limit)
    {}
    virtual ~stderr_transport() {}
};

}// namespace httptth

#endif // __LOGGER_FILE_H__
