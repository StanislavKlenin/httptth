#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

#include <exception>

namespace endpoint_reloaded
{

// skeletal exception hierarchy, needs some work

class generic_exception : public std::exception
{
public:
    const char* what() const noexcept override
    {
        return "generic_exception";
    }
};

class overflow_error : public generic_exception
{
public:
    const char* what() const noexcept override
    {
        return "overflow_error";
    }
};

class network_exception : public generic_exception
{
public:
    const char* what() const noexcept override
    {
        return "network_exception";
    }
};

class io_exception : public generic_exception
{
public:
    const char* what() const noexcept override
    {
        return "io_exception";
    }
};

class read_error : public io_exception
{
public:
    const char* what() const noexcept override
    {
        return "read_error";
    }
};

class write_error : public io_exception
{
public:
    const char* what() const noexcept override
    {
        return "write_error";
    }
};



} // namespace endpoint_reloaded

#endif // __EXCEPTIONS_H__
