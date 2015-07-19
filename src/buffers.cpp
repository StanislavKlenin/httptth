#include "buffers.hpp"

#include <string.h> // memcpy

namespace httptth
{

constexpr size_t write_buffer::capacity;
constexpr size_t read_buffer::capacity;

void write_buffer::write(const char *data, size_t length)
{
    if (p + length < buffer + sizeof(buffer)) {
        // data fits without the need to flush
        memcpy(p, data, length);
        p += length;
    } else {
        // not enough room
        while (length) {
            flush(); // may throw
            
            // copy a chunk
            size_t chunk = std::min(length, capacity);
            memcpy(p, data, chunk);
            p += chunk;
            data += chunk;
            length -= chunk;
        }
    }
}

readable::status read_buffer::get_char(char &c)
{
    if (depleted()) {
        if (eof()) {
            return status::eof;
        } else {
            return status::again;
        }
    } else {
        c = buffer[returned];
        returned++;
        return status::ok;
    }
}

} // namespace httptth
