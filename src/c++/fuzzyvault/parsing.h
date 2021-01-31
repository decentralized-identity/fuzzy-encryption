/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#ifndef _PARSING_H_
#define _PARSING_H_

#include <stdint.h>
#include <vector>
#include "json.h"

/// Converts a JSON string of the form "HHHHHH" where H is an upper
/// case hexadecimal digit into an array of bytes
/// @param E a json object determined to be a string
/// @param dst the destination array of bytes
/// @returns void
void json_read_bytes(
    const json_object_element_s* E,
    std::vector<uint8_t>& dst
    );

/// Converts a JSON array containing strings 
/// of the form "HHHHHH" where H is an upper
/// case hexadecimal digit into an array of bytes
/// @param E a json object determined to be a string
/// @param dst the destination array of bytes
/// @returns void
void json_read_bytes_ex(
    const json_object_element_s* E,
    std::vector<uint8_t>& dst
    );


/// Converts an json object of type int into an integer
/// Calls json_read_int_value()
/// @param E a json object of type int
/// @param dst the destination integer to get the value
/// @returns void
void json_read_int(
    const json_object_element_s* E,
    int& dst
    );

/// An internal function called by json_read_int()
/// @param V a json value of type int
/// @param dst the place to write the integer value
/// @returns void
void json_read_int_value(const json_value_s* V, int& dst);

/// Reads an array of integers into a list
/// @param E a json object element
/// @param dst the list of integers to be filled
/// @returns void
void json_read_ints(const json_object_element_s* E, std::vector<int>& dst);

#endif
