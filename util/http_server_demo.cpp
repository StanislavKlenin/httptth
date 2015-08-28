#include "http_server.hpp"

using namespace std;
using namespace httptth;

void test_http_server(const char *host, int port)
{
    auto http_handler = [](http_server::request const &req,
                           http_server::response      &res)
    {
        dprintf(logger::INFO, "req %s %s", req.method.c_str(), req.url.c_str());
        try {
            // add new header (schedule it to be written later):
            res.add_header("Content-Type", "text/plain");
            
            // send the header right away
            // (forcing status line and other headers to be sent as well):
            res.write_header("Connection", "keep-alive");
            
            // write something as a body
            res << "YO\n";
        } catch (generic_exception &e) {
            dprintf(logger::NOTICE, "exception in handler: %s", e.what());
        }
    };
    //http_server::handler_type f(tmp_handler);
    //http_server http(f);
    //http.listen(nullptr, 1234);
    
    //http_server http(tmp_handler);
    
    dprintf(logger::INFO,
            "starting server at %s:%d",
            host ? host : "localhost",
            port);
    
    http_server http(http_handler);
    http.listen(host, port);
}

int main()
{
    test_http_server(nullptr, 1234);
    return 0;
}
