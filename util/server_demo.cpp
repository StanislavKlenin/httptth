#include <signal.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "core/connection.hpp"
#include "core/text_server.hpp"
#include "core/http_server.hpp"

using namespace std;
using namespace httptth;

/*
struct concept
{
    void operator()(const char *data, size_t length, writable &destination) {
        destination.write(data, length);
        destination.flush();
        if (strncmp("close\r\n", data, length) == 0) {
            destination.end();
        }
    }
    bool operator()(const char *s, size_t l) { return s[l - 1] == '\n'; }
};
void test_concept()
{
    concept c;
    another_server<concept> concept_server(c);
    concept_server.listen(nullptr, 1234);
    //concept_server.ignorethis(42);
}
*/

//void signal_handler(int signo, siginfo_t * /*info*/, void * /*context*/)
//{
//    if (signo == SIGINT || signo == SIGTERM) {
//        fprintf(stderr, "signal %d\n", signo);
//        if (server_ptr) {
//            server_ptr->stop();
//        }
//    }
//}

//------------------------------------------------------------------------------
void test_http_server()
{
    
    
    auto http_handler = [](http_server::request const &req,
                           http_server::response      &res)
    {
        //fprintf(stderr, "http handler\n");
        dprintf(logger::DEBUG, "http handler\n");
        //sleep(5);
        //res.headers.emplace_back("Content-Type", "text/plain");
        res << "YO\n";
    };
    //http_server::handler_type f(tmp_handler);
    //http_server http(f);
    //http.listen(nullptr, 1234);
    
    //http_server http(tmp_handler);
    http_server http(std::ref(http_handler));
    http.listen(nullptr, 1234);
}
//------------------------------------------------------------------------------

bool custom_trigger(const char *data, size_t length)
{
    return length && data[length - 1] == '|';
}

void test_text_server()
{
    auto echo_handler = [](const char *data,
                           size_t length,
                           writable &destination)
    {
        // write each line back to the client
        destination.write(data, length);
        // flush is needed to send the data right away
        // (and not when buffer gets full)
        destination.flush();
        
        if (strncmp("close\r\n", data, length) == 0) {
            destination.end();
        }
    };
    
    line_feed_text_server server(echo_handler);
    server.listen(nullptr, 1234);
}

int main(int argc, char **argv)
{
    
    
    //fprintf(stderr, "%d\n", connection::new_line_trigger("as\n", 3)); 
    
    //test_text_server(); return 0;
    test_http_server(); return 0;
    
    /*
    
    auto echo_handler = [](const char *data,
                           size_t length,
                           writable &destination)
    {
        // write each line back to the client
        destination.write(data, length);
        // flush is needed to send the data right away
        // (and not when buffer gets full)
        destination.flush();
        
        if (strncmp("close\r\n", data, length) == 0) {
            destination.end();
        }
    };
    text_server<decltype(echo_handler)> server(echo_handler);
    //text_server server(echo_handler, custom_trigger);
    
    //text_server::handler_type f(std::ref(echo_handler));
    ////text_server server(f);
    //text_server server(f, custom_trigger);
    
    server.listen(nullptr, 1234);
    
    */
}

// yet another overhaul
/*

suppose we have a server that we can use as before:
auto handler = [](const char *s, size_t l, writable &dst);
text_server server(handler);

BUT it can also be constructed like this:
class custom_handler : public text_server::generic_handler {
    // this will be called on every message
    void operator()(const char *s, size_t l, writable &dst) override;
    // and this checks buffer content and tells if it is a message
    bool trigger(const char *, size_t) override;
};

internally, text_server constructs its own generic_handler from std::function
and passes trigger() to connection constructor


*/



