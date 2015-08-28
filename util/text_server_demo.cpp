#include <string.h> // strncmp

#include "text_server.hpp"

using namespace std;
using namespace httptth;

void test_text_server(const char *host, int port)
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
    
    dprintf(logger::INFO,
            "starting server at %s:%d",
            host ? host : "localhost",
            port);
    
    line_feed_text_server server(echo_handler);
    server.listen(host, port);
}

int main()
{
    test_text_server(nullptr, 1234);
    return 0;
}
