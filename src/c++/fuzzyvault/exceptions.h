#ifndef _EXCEPTIONS_H_  
#define _EXCEPTIONS_H_

#include <exception>

/// This is the standard exception that we throw
/// when something is terribly wrong and we
/// terminate the process
class Exception : public std::exception
{
    const char* _msg;
public:
    Exception() {}
    Exception(const char* msg) : _msg(msg) {}
    const char* what() const throw() { return _msg; }
};

#endif