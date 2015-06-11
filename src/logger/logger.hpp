#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <cstring>

#include <memory>
#include <vector>

namespace httptth
{

// namespace log // ?

class transport;

class logger
{
public:
    enum level
    {
        EMERG   = 0, // system is unusable
        ALERT   = 1, // action must be taken immediately
        CRIT    = 2, // critical conditions
        ERR     = 3, // error conditions
        WARNING = 4, // warning conditions
        NOTICE  = 5, // normal but significant condition
        INFO    = 6, // informational
        DEBUG   = 7  // debug-level messages
    };
    
public:
    logger() {}
    explicit logger(std::unique_ptr<transport> t);
    
    void log(enum level level, const char *data, size_t length);
    
    inline void log(enum level level, const char *data)
    {
        log(level, data, std::strlen(data));
    }
    
    template<typename ...A>
    void format(enum level level, const char *format, A ...args);
    
    void add_transport(std::unique_ptr<transport> /*t*/);
    
    template<typename T, typename ...A>
    void add_transport(A ...args);
    
public:
    static const char *level_names[];
    
private:
    std::vector<std::unique_ptr<transport> > transports;
};

class transport // abstract
{
public:
    using pointer = std::unique_ptr<transport>;
    
public:
    virtual ~transport() {}
    
    // perform actual logging  
    virtual void log(logger::level level, const char *data, size_t length) = 0;
    
    // printf-like logging
    virtual void format(logger::level level, const char *format, ...) = 0;
    
    // check if this transport supports messages of this level
    virtual bool supports(logger::level level) const = 0;
    
public:
    // "statically polymorphic" factory method
    template<typename T, typename ...A>
    static pointer make(A ...args)
    {
        return pointer(new T(std::forward<A>(args)...));
    }
};

template<typename T, typename ...A>
void logger::add_transport(A ...args)
{
    add_transport(transport::make<T>((args)...));
}

template<typename ...A>
void logger::format(logger::level level, const char *format, A ...args)
{
    for (auto &t : transports) {
        if (t->supports(level)) {
            t->format(level, format, std::forward<A>(args)...);
        }
    }
}

} // namespace httptth

#endif // __LOGGER_H__
