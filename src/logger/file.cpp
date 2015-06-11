#include "file.hpp"

#include <stdarg.h>

#include <stdexcept>

using namespace std;

namespace httptth
{

file_transport::file_transport(const char *filename, logger::level limit) :
    priority_limited_transport(limit),
    file(nullptr),
    owner(true)
{
    if (filename) {
        file = fopen(filename, "at");
    } else {
        throw invalid_argument("filename is null");
    }
    if (!file) {
        throw invalid_argument(filename);
    }
    
    setbuf(file, NULL);
}

file_transport::~file_transport()
{
    if (owner) {
        fclose(file);
    }
}

void file_transport::log(logger::level level, const char *data, size_t length)
{
    fprintf(file, "%s %.*s\n", logger::level_names[level], (int)length, data);
}

void file_transport::format(logger::level level, const char *format, ...)
{
    fmt.assign(logger::level_names[level]);
    fmt.append(1, ' ');
    fmt.append(format);
    fmt.append(1, '\n');
    
    va_list args;
    va_start(args, format);
    vfprintf(file, fmt.c_str(), args);
    va_end(args);
}

} // namespace httptth
