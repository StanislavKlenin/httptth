#ifdef DEBUG_LOG_LEVEL

#include "debug.hpp"

namespace httptth
{

beamer::logger static_logger(
    beamer::transport::make<beamer::stderr_transport>(
        static_cast<beamer::logger::level>(DEBUG_LOG_LEVEL)));

} // namespace httptth

#endif
