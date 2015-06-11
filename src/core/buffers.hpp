#ifndef __BUFFERS_H__
#define __BUFFERS_H__

#include <stddef.h>

#include "exceptions.hpp"
#include "io.hpp"

#include <algorithm> // min

namespace httptth
{

// fixed-sized memory buffer for writing
// partial implementation of writable interface,
// without any definition of underlying data reciever
class write_buffer : public writable
{
public:
    static constexpr size_t capacity = 1024 * 64;
    //static constexpr size_t capacity = 16;
    
    write_buffer() : p(buffer) {}
    virtual ~write_buffer() {}
    
    // rewind without writing, effectively discarding all buffer content
    inline void reset() { p = buffer; }
    
    void write(const char *data, size_t length) override;
    
protected:
    inline const char *data() const { return buffer; }
    inline const size_t length() const { return p - buffer; }
    
private:
    char  buffer[capacity];
    char *p;
    // TODO: different way to define a const
};

// fixed-sized memory buffer for reading
// partial implementation of readable interface
// without any definition of underlying data source
class read_buffer : public readable
{
public:
    static constexpr size_t capacity = 1024 * 64;
    
public:
    read_buffer() : retrieved(0), returned(0) {}
    virtual ~read_buffer() {}
    
    // read some data from the source (source is defined by implementation)
    // "flush reversed"
    virtual readable::status fill() = 0;
    
    // eof: there is no more data in the source AND in the buffer
    // (eof implies depleted)
    virtual bool eof() const = 0;
    
    // depleted: all data in the buffer has been returned, need to fill again
    inline bool depleted() const { return returned >= retrieved; }
    
    // fetch single byte, if possible
    readable::status get_char(char &c);
    
protected:
    // write access to the buffer for the implementers
    inline char *data() { return buffer; }
    // mark some part of the buffer as retrieved (setter for retrieved field)
    inline void claim(size_t retrieved)
    {
        this->retrieved = std::min(retrieved, capacity);
    }
    inline void rewind() { returned = 0; }
    
private:
    char   buffer[capacity];
    size_t retrieved; // from the source
    size_t returned;  // to the client
};

} // namespace httptth

#endif // __BUFFERS_H__
