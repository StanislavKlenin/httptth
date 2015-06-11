#ifndef __LOGGER_COMMON_H__
#define __LOGGER_COMMON_H__

#include "logger.hpp"

namespace httptth
{

class priority_limited_transport : public transport
{
public:
    priority_limited_transport(logger::level limit = logger::DEBUG) :
        log_limit(limit)
    {}
    virtual ~priority_limited_transport() {}
    
    bool supports(logger::level level) const override
    {
        return level <= log_limit;
    }
    
private:
    logger::level log_limit;
};

}// namespace httptth

#endif // __LOGGER_COMMON_H__
