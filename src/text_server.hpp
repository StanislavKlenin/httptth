#ifndef __TEXT_SERVER_H__
#define __TEXT_SERVER_H__

#include "connection.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <string.h> // memset
#include <unistd.h>

#include <thread>

namespace httptth
{

class scoped_fd
{
public:
    explicit scoped_fd(int descriptor) : fd(descriptor) {}
    ~scoped_fd() { close(fd); }
    scoped_fd(const scoped_fd &) = delete;
    scoped_fd(scoped_fd &&that) : fd(that.fd) { that.fd = -1; }
    scoped_fd &operator =(const scoped_fd &) = delete;
    
    inline operator int() { return fd; }
private:
    int fd;
};

// generic line-based text server
template<typename handler>
class text_server
{
public:
    explicit text_server(const handler &h) :
        prototype(h),
        stop_requested(false)
    {}
    explicit text_server(handler &&h) :
        prototype(std::move(h)),
        stop_requested(false)
    {}
    text_server(const text_server &) = delete;
    text_server &operator =(const text_server &) = delete;
    text_server(const text_server &&) = delete;
    text_server &operator =(const text_server &&) = delete;
    
    void listen(const char *address, int port);
    inline void stop() { stop_requested = true; }
    
protected:
    struct context
    {
        handler    h;
        connection conn;
        // context must always copy the handler
        context(int fd, const handler &hdl) :
            h(hdl),
            conn(fd, ([&](const char *s, size_t l) { return h(s, l); }))
        {}
    };
    
private:
    handler prototype;
    bool    stop_requested;
    static const time_t timeout = 5;
    
    void process_client(int fd);
    
};

template<typename handler>
void text_server<handler>::listen(const char *address, int port)
{
    sockaddr_in addr;
    
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    
    if (address) {
        int status = inet_pton(AF_INET, address, &addr.sin_addr);
        if (status <= 0) {
            throw generic_exception();
        }
    } else {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    
    scoped_fd listenfd(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (listenfd < 0) {
        perror("socket");
        throw network_exception();
    }
    
    int e = 1;
    int rc = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &e, sizeof e);
    if (rc < 0) {
        dprintf(logger::ERR, "setsockopt");
        throw network_exception();
    }
    
    rc = bind(listenfd, (sockaddr *)&addr, sizeof(addr));
    if (rc < 0) {
        perror("bind");
        throw network_exception();
    }
    
    rc = ::listen(listenfd, 128);
    if (rc < 0) {
        perror("listen");
        throw network_exception();
    }
    
    // trivial event loop
    // later: init "listen thread" and detach it
    struct timeval tv;
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
    
    stop_requested = false;
    
    while (true) {
        // TODO: loop exit condition
        if (stop_requested) {
            // possibly log something later
            break;
        }
        
        sockaddr_in client_addr;
        socklen_t l = sizeof(client_addr);
        int fd = accept(listenfd, (sockaddr *)&client_addr, &l);
        if (fd < 0) {
            perror("accept failed");
            continue;
        }
        
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
        // status: read() times out, but this is not handled yet
        
        std::thread t(&text_server<handler>::process_client, this, fd);
        t.detach();
    } // while (true
}

template<typename handler>
void text_server<handler>::process_client(int fd)
{
    try {
        dprintf(logger::DEBUG, "client fd=%d", fd);
        context ctx(fd, prototype);
        
        // bind the third parameter (or use std::bind instead)
        auto read_handler = [&ctx](const char *line, size_t length) {
            dprintf(logger::DEBUG, "\\(l=%zu) %.*s", length, (int)length, line);
            ctx.h(line, length, ctx.conn);
        };
        readable::handler_type f(std::ref(read_handler));
        
        // actually, that could be enough:
        // while (ctx.conn.read_line(f) == readable::status::ok) ;
        
        while (true) {
            readable::status status = ctx.conn.read_line(f);
            
            if (status == readable::status::eof) {
                break;
            }
            
            if (status == readable::status::again) {
                // consider this a timeout
                // a problem: some leftover bytes might still be in the buffer
                // but do we care, as we are about to disconnect this client?
                dprintf(logger::DEBUG, "client %d timed out", fd);
                break;
            }
            
        } // while (true)
        dprintf(logger::DEBUG, "client fd=%d: done", fd);
        // close (and therefore flush) should be called automatically
    } catch (generic_exception &e) {
        dprintf(logger::WARNING, "excpetion: %s", e.what());
    } catch (std::exception &e) {
        dprintf(logger::WARNING, "std::excpetion: %s", e.what());
    }
}

// default implementation (with triggering on each new line)

// a handler that is a pure function, without a trigger
using stateless_handler = std::function<void(const char *, size_t, writable &)>;

// handler_type implementation that triggers client function on each new line
struct on_new_line
{
    on_new_line(const stateless_handler &f) : h(f) {}
    on_new_line(stateless_handler &&f) : h(std::move(f)) {}
    void operator()(const char *s, size_t l, writable &dst) { h(s, l, dst); }
    bool operator()(const char *s, size_t l) { return s[l - 1] == '\n'; }
    stateless_handler h;
};

// text server with on_new_line handler
class line_feed_text_server : public text_server<on_new_line> {
public:
    line_feed_text_server(stateless_handler h) :
        text_server<on_new_line>(on_new_line(h))
    {}
};

} // namespace httptth

#endif // __TEXT_SERVER_H__
