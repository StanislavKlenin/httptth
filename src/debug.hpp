#ifndef __HTTPTTH_DEBUG_H__
#define __HTTPTTH_DEBUG_H__



//#define DEBUG_LOG_LEVEL 7

#ifdef DEBUG_LOG_LEVEL
    #include "beamer/src/logger.hpp"
    #include "beamer/src/file.hpp"
    namespace httptth
    {
    
    extern beamer::logger static_logger;
    
    #define dprintf(level, ...) \
        do { static_logger.format(level, __VA_ARGS__); } while (0)
    
    } // namespace httptth
#else
    namespace httptth
    {
    
    #define dprintf(level, ...) \
        do {} while (0)
    
    } // namespace httptth
#endif

#endif // __HTTPTTH_DEBUG_H__
