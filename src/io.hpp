#ifndef __IO_H__
#define __IO_H__

#include <stddef.h>

#include <functional>
#include <string>

namespace endpoint_reloaded
{

// some core concepts

class readable // abstract / interface
{
public:
    enum class status
    {
        again = -1,
        eof   =  0,
        ok    =  1
    };
    
public:
    virtual ~readable() {}
    
    using handler_type = std::function<void(const char *, size_t)>;
    
    // read some chank of data and pass it to the handler
    virtual status read_line(handler_type &handler) = 0;
    
    // stop reading (close)
    virtual void enough() = 0;
};

class writable // abstract / interface
{
public:
    virtual ~writable() {}
    
    // consume some data, possibly storing some of it in a buffer
    virtual void write(const char *data, size_t length) = 0;
    
    // perform actual writing, commiting buffered data to the underlying system
    virtual void flush() = 0;
    
    // stop writing (close)
    virtual void end() = 0;
    
    // maybe:
    // request status, "closed", "connected", "write_possible" "online"?
};

writable & operator << (writable &destination, unsigned n);
writable & operator << (writable &destination, char c);
writable & operator << (writable &destination, const char *s);
writable & operator << (writable &destination, const char *s);
writable & operator << (writable &destination, const std::string &s);

} // namespace endpoint_reloaded

#endif // __IO_H__
