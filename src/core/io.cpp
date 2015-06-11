#include "io.hpp"

#include <string.h>

namespace httptth
{

writable & operator << (writable &destination, unsigned n)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%u", n);
    return (destination << buffer);
}

writable & operator << (writable &destination, char c)
{
    destination.write(&c, 1);
    return destination;
}

writable & operator << (writable &destination, const char *s)
{
    destination.write(s, strlen(s));
    return destination;
}

writable & operator << (writable &destination, const std::string &s)
{
    destination.write(s.c_str(), s.length());
    return destination;
}

} // namespace httptth
