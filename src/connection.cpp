#include "connection.hpp"

#include <sys/socket.h> // shutdown
#include <unistd.h>

namespace httptth
{

connection::connection(int descriptor, const connection::trigger_condition c) :
    read_buffer(),
    write_buffer(),
    fd(descriptor),
    trigger(c),
    eof_flag(false),
    read_flag(true),
    write_flag(true)
{
    dprintf(logger::DEBUG, "connection constructed for fd=%d", fd);
}

connection::connection(connection &&that) :
    read_buffer(that),
    write_buffer(that),
    fd(that.fd),
    trigger(std::move(that.trigger)),
    eof_flag(that.eof_flag),
    read_flag(that.read_flag),
    write_flag(that.write_flag)
{
    that.fd = -1;
    that.read_flag = that.write_flag = false;
}

connection::~connection()
{
    // close reading part:
    enough();
    // close writing part:
    end();
    // release socket resources:
    close(fd);
    // note: closing fd will initiate TCP shutdown anyway
    //       so enough() and end() are redundant here
}

//void connection::close()
//{
//    if (!closed()) {
//        flush();
//        fprintf(stderr, "closing fd %d\n", fd);
//        ::close(fd);
//        fd = 0;
//    }
//}

read_buffer::status connection::fill()
{
    if (eof_flag) {
        return status::eof;
    }
    
    do {
        auto cnt = read(fd, read_buffer::data(), read_buffer::capacity);
        auto err = errno;
        if (cnt < 0) {
            // error
            if (err == EINTR) {
                // interrupted, try again
                continue;
            } else if (err == EAGAIN || err == EWOULDBLOCK) {
                return status::again;
            } else {
                throw read_error();
                // TODO: pass some params to the exception?
            }
        } else if (cnt == 0) {
            // eof
            eof_flag = true;
            claim(0);
            return status::eof;
        } else {
            // ok
            claim(cnt);
            rewind();
            return status::ok;
        }
    } while (true);
}

readable::status connection::read_line(readable::handler_type &handler)
{
    // action: get_char or fill
    bool data_needed = false;
    while (true) {
        
        // TODO: if (!read_flag) ...
        
        // check handler call possibility
        if (!buffer.empty() && trigger(&buffer[0], buffer.size())) {
            handler(&buffer[0], buffer.size());
            buffer.clear();
            return status::ok;
        }
        
        if (data_needed) {
            //dprintf(logger::DEBUG, "data needed branch, calling fill");
            // call fill
            auto rc = fill();
            switch (rc) {
                case status::again:
                    // most interesting case, fill could not complete
                    // consider this equivalent to eof?
                    // or propagate to the caller?
                    //dprintf(logger::DEBUG,
                    //        "fill: status=again with %zu in the buffer",
                    //        buffer.size());
                    return rc;
                    break;
                case status::eof:
                    // if fill returned eof, next get_char() will too
                    break;
                default:
                     // fill succeeded, do nothing
                    ;
            } // switch
            data_needed = false;
        } else {
            //dprintf(logger::DEBUG,
            //        "data not needed branch, calling get_char");
            // call get_char
            char c;
            auto rc = get_char(c);
            switch (rc) {
                case status::again:
                    // raise data needed flag
                    data_needed = true;
                    break;
                case status::eof:
                    // call handler on the data we have
                    // (even if not complete line/chunk)
                    handler(&buffer[0], buffer.size());
                    buffer.clear();
                    return status::eof;
                default:
                    buffer.push_back(c);
                    // copied one byte, continue with our loop
            } // switch
        }
    } // while
    
    return status::eof; // should be unreachable
}

void connection::enough()
{
    if (read_flag) {
        ::shutdown(fd, SHUT_RD);
        // TODO: check status
        read_flag = false;
    }
}

void connection::flush()
{
    if (!write_flag) return;
    dprintf(logger::DEBUG, "flush: %zu bytes", length());
    if (length()) {
        ssize_t written = 0;
        auto q = write_buffer::data();
        
        do {
            written = ::write(fd, q, write_buffer::data() + length() - q);
            if (written < 0) {
                if (errno == EINTR) {
                    continue;
                }
                throw write_error();
            } else {
                q += written;
            }
        } while (q < write_buffer::data() + length());
        
        reset();
    }
}

void connection::end()
{
    if (write_flag) {
        // flush?
        
        ::shutdown(fd, SHUT_WR);
        // TODO: check status
        write_flag = false;
    }
}

const connection::trigger_condition connection::on_new_line =
    [](const char *d, size_t l)
    {
        return (l && d[l - 1] == '\n');
    };

} // namespace httptth
