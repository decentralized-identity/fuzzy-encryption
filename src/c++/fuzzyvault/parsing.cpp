/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#include <iostream>
#include <stdlib.h>
#include "parsing.h"
#include "exceptions.h"

static int char_to_hex(char c)
{
    if ('0' <= c && c <= '9')
        return c - '0';
    else if ('A' <= c && c <= 'F')
        return c - 'A' + 10;
    else
        throw Exception("invalid value for c");
    return 0;
}

void json_read_bytes(const json_object_element_s* E, std::vector<uint8_t>& dst)
{
    if (E == 0)
        throw Exception("json_read_bytes -- E == 0");
    if (E->value == 0)
        throw Exception("json_read_bytes -- E->value == 0");
    if (E->value->type != json_type_string)
        throw Exception("json_read_bytes -- E->value->type != json_type_string");
    if (E->value->payload == 0)
        throw Exception("json_read_bytes -- E->value->payload == 0");
    const json_string_s& src = *((const json_string_s*)E->value->payload);
    if (src.string_size < 2 || src.string_size %  2 != 0)
        throw Exception("json_read_bytes");
    const size_t n_bytes = src.string_size / 2;
    dst.resize(n_bytes);
    for (size_t i = 0; i < n_bytes; i++)
    {
        const char c_hi = src.string[2*i];
        const char c_lo = src.string[2*i + 1];
        const int n_hi = char_to_hex(c_hi);
        const int n_lo = char_to_hex(c_lo);
        const uint8_t x = (uint8_t)((n_hi << 4) + n_lo);
        dst[i] = x;
    }
}

void json_read_bytes_ex(const json_object_element_s* E, std::vector<uint8_t>& dst)
{
    if (E == 0)
        throw Exception("json_read_bytes_ex -- E == 0");
    if (E->value == 0)
        throw Exception("json_read_bytes_ex -- E->value == 0");
    if (E->value->type != json_type_array)
        throw Exception("json_read_bytes_ex -- E->value->type != json_type_array");
    if (E->value->payload == 0)
        throw Exception("json_read_bytes_ex -- E->value->payload == 0");
    const json_array_s& array = *((const json_array_s*)E->value->payload);
    if (array.length == 0)
        throw Exception("json_read_bytes_ex -- array.length == 0");
    if (array.start == 0)
        throw Exception("json_read_bytes_ex -- array.start == 0");
    for (struct json_array_element_s* ae = array.start; ae != 0; ae = ae->next)
    {
        if (ae->value == 0)
            throw Exception("json_read_bytes_ex -- ae.value == 0");
        if (ae->value->type != json_type_string)
            throw Exception("json_read_bytes_ex -- ae->value->type != json_type_string");
        if (ae->value->payload == 0)
            throw Exception("json_read_bytes_ex -- ae->value->payload == 0");
        const json_string_s& src = *((const json_string_s*)ae->value->payload);
        if (src.string_size < 2 || src.string_size %  2 != 0)
            throw Exception("json_read_bytes");
        const size_t n_bytes = src.string_size / 2;
        for (size_t i = 0; i < n_bytes; i++)
        {
            const char c_hi = src.string[2*i];
            const char c_lo = src.string[2*i + 1];
            const int n_hi = char_to_hex(c_hi);
            const int n_lo = char_to_hex(c_lo);
            const uint8_t x = (uint8_t)((n_hi << 4) + n_lo);
            dst.push_back(x);
        }
    }
}

void check_json_element(const json_object_element_s* E, json_type_e json_type)
{
    if (E == 0 || E->name == 0 || E->value == 0 || E->value->type != json_type || E->value->payload == 0)
        throw Exception("check_json_element -- bad object");
}

void json_read_int(const json_object_element_s* E, int& dst)
{
    check_json_element(E, json_type_number);
    json_read_int_value(E->value, dst);
}

void json_read_int_value(const json_value_s* V, int& dst)
{
    if (V == 0 || V->type != json_type_number || V->payload == 0)
        throw Exception("json_read_int_value -- bad value");
    const json_number_s* N = (const json_number_s*)V->payload;
    if (N->number == 0)
        throw Exception("json_read_int: N->number == 0");
    dst = atoi(N->number);
}

void json_read_ints(const json_object_element_s* obj, std::vector<int>& dst)
{
    check_json_element(obj, json_type_array);
    const json_array_s& src = *((const json_array_s*)obj->value->payload);
    dst.resize(src.length);
    int i = 0;
    for (const json_array_element_s* A = src.start; A; A = A->next, i++)
    {
        json_read_int_value(A->value, dst[i]);
    }
}
