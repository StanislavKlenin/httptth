#include "text_server.hpp"
#include "exceptions.hpp"

#include <arpa/inet.h>
#include <errno.h>
#include <string.h> // memset
#include <unistd.h>

#include <thread>

using namespace std;

namespace endpoint_reloaded
{
/*
//
// text_server
//

text_server::text_server(text_server::handler_type h,
                         connection::trigger_condition c) :
    handler(std::unique_ptr<extended_handler>(new lambda_handler(h, c)))
{
    fprintf(stderr, "text_server constructed (from handler_type)\n");
}

text_server::text_server(text_server::extended_handler_ptr h) :
    handler(std::move(h))
{
    fprintf(stderr, "text_server constructed (from extended_handler)\n");
}

void text_server::listen(const char *address, int port)
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
    
    // implementation kicks in?
    // or implement it right here?
    
    auto listenfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenfd < 0) {
        perror("socket");
        throw network_exception();
    }
    
    char optval = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval);
    
    int rc = bind(listenfd, (sockaddr *)&addr, sizeof(addr));
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
    
    while (true) {
        sockaddr_in client_addr;
        socklen_t l = sizeof(client_addr);
        int fd = accept(listenfd, (sockaddr *)&client_addr, &l);
        if (fd < 0) {
            perror("accept failed");
            continue;
        }
        
        setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));
        // status: read() times out, but this is not handled yet
        
        thread t(&text_server::process_client, this, fd);
        t.detach();
        
    } // while (true)
    
    // loop must be interrupted to get here
    ::close(listenfd);
}

void text_server::process_client(int fd)
{
    try {
        fprintf(stderr, "client fd=%d\n", fd);
        // TODO; pass NEW trigger generated from handler for erach context
        //context ctx(fd, handler, std::ref(generator));
        //context ctx(fd, handler, std::ref(trigger));
        context ctx(fd, std::move(handler->clone()));
        fprintf(stderr, "after context constructed\n");
        
        auto read_handler = [&ctx](const char *line, size_t length) {
            fprintf(stderr, "\\(l=%zu): %.*s\n", length, (int)length, line);
            dump_data(line, length);
            //ctx.handler(line, length, ctx.conn);
            ctx.handler->process(line, length, ctx.conn);
        };
        fprintf(stderr, "after lambda constructed\n");
        readable::handler_type f(std::ref(read_handler));
        
        fprintf(stderr, "after read handler constructed\n");
        
        // maybe: std::bind version
        
        // actually, that could be enough:
        // while (ctx.conn.read_line(f)) ;
        
        while (true) {
            readable::status status = ctx.conn.read_line(f);
            fprintf(stderr, "[fd=%d] read_line returned %d\n", fd, status);
            
            if (status == readable::status::eof) {
                break;
            }
            
            if (status == readable::status::again) {
                // consider this a timeout
                // a problem: some leftover bytes might still be in the buffer
                // but do we care, as we are about to disconnect this client?
                fprintf(stderr, "client %d timed out\n", fd);
                break;
            }
        }
        fprintf(stderr, "client fd=%d: done\n", fd);
        // close (and therefore flush) should be called automatically
    } catch (generic_exception &e) {
        fprintf(stderr, "excpetion: %s\n", e.what());
    } catch (std::exception &e) {
        fprintf(stderr, "std::excpetion: %s\n", e.what());
    }
}


void text_server::dump_data(const char *data, size_t length)
{
    for (size_t i = 0; i < length; i++) {
        fprintf(stderr, "%02x", data[i]);
    }
    fprintf(stderr, "\n");
}
*/
} // namespace endpoint_reloaded
