/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#ifndef _TYPES_H_
#define _TYPES_H_

#include <stdint.h>
#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include "json.h"

#define countof(_X_) (sizeof(_X_) / sizeof(_X_[0]))


/// for printing list of integers in JSON format
std::ostream& operator<<(std::ostream& os, const std::vector<int>& xs);

/// for printing list of bytes in JSON format
std::ostream& operator<<(std::ostream& os, const std::vector<uint8_t>& xs);

/// Reads the contents of a file into a string
/// @param path the path to the text file
/// @return a string containing the contents of the text file
std::string loads(const std::string& path);

/// Internal function that appends an int converted to bytes to 
/// a list of bytes
/// @param t the integer to be converted to bytes and appended
/// @param out the list of bytes to be appended to
void pushback_int(
    int t,
    std::vector<uint8_t>& out
    );

#endif
