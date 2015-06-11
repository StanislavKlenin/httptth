#include "logger/logger.hpp"
#include "logger/file.hpp"

using namespace std;
using namespace httptth;


int main(int argc, char **argv)
{
    //stderr_transport t;
    //t.format(logger::DEBUG, "test: %d", 4);
    
    class logger logger(transport::make<stderr_transport>(logger::CRIT));
    
    logger.add_transport<stderr_transport>();
    logger.log(logger::ERR, "error");
    logger.log(logger::CRIT, "critical");
    
    return 0;
}
