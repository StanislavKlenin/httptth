#include <string.h> // strncmp

#include "text_server.hpp"

using namespace std;
using namespace httptth;

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
    test_text_server();
    return 0;
}
