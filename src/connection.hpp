#ifndef __CONNECTION_H__
#define __CONNECTION_H__

#include "io.hpp"
#include "buffers.hpp"

#include <vector>

namespace httptth
{

// an entity that is both readable and writable,
// built around a socket descriptor
// using multiple implementation inheritance (this should be reconsidered)
// some name clashes ensued
class connection : public read_buffer, public write_buffer
{
public:
    // trigger condition: a predicate taking current buffer contents
    // it must make a decision based on the contents (or something else)
    // and return true if a handler must be called
    using trigger_condition = std::function<bool(const char *, size_t)>;
    
    static const trigger_condition on_new_line;
    
public:
    // valid socket descriptor required
    connection(int descriptor, trigger_condition c = on_new_line);
    connection(const connection &) = delete;
    connection &operator =(const connection &) = delete;
    connection(connection &&that);
    virtual ~connection();
    
    //inline bool closed() const { return (fd == 0); }
    
    // from read_buffer
    read_buffer::status fill() override;
    bool eof() const override { return eof_flag; }
    readable::status read_line(readable::handler_type &handler) override;
    void enough() override;
    
    // from write_buffer
    void flush() override;
    void end() override;
    
private:
    int                fd;
    trigger_condition  trigger;
    bool               eof_flag, read_flag, write_flag;
    std::vector<char>  buffer;
    
};

} // namespace httptth

#endif // __CONNECTION_H__
