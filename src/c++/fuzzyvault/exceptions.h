/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

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