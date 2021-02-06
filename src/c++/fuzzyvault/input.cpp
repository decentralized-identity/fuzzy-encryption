/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#include "input.h"
#include "types.h"
#include "parsing.h"
#include "exceptions.h"

std::ostream& operator<<(std::ostream& os, const input_t& input)
{
    os  << "{" << std::endl
        << "  \"setSize\": " << std::dec << input._setSize << "," << std::endl
        << "  \"corpusSize\": " << std::dec << input._corpusSize << "," << std::endl
        << "  \"correctThreshold\": " << std::dec << input._correctThreshold << std::endl
        << "}";
    return os;
}

input_t::input_t(const std::string& json)
{
    const std::string setSize_s("setSize");
    const std::string corpusSize_s("corpusSize");
    const std::string correctThreshold_s("correctThreshold");
    const std::string randomBytes_s("randomBytes");
    struct json_value_s* root = json_parse(json.c_str(), json.length());

    bool set_setSize = false;
    bool set_corpusSize = false;
    bool set_correctThreshold = false;
    bool set_randomBytes = false;

    if (root == 0)
        throw Exception("input_t:input_t -- json_parse failed");
    try
    {
        if (root->type != json_type_object)
            throw Exception("input_t::input_t -- JSON does not represent an object");
        json_object_s* object = (json_object_s*)root->payload;
        if (object == 0)
            throw Exception("input_t::input_t -- root->payload is null");
        json_object_element_s* E = object->start;
        if (object->length != 3 && object->length != 4)
            throw Exception("input_t::input_t -- bad number of key-value pairs");
        for (size_t i = 0; i < object->length; i++, E = E->next) 
        {
            const char* name = E->name->string;
            if (name == 0)
                throw Exception("input_t::input_t -- name == 0");
            if (setSize_s.compare(name) == 0) 
            {
                if (set_setSize)
                    throw Exception("input_t::input_t -- setSize set more than once");
                json_read_int(E, _setSize);
                set_setSize = true;
            }
            else if (corpusSize_s.compare(name) == 0)
            {
                if (set_corpusSize)
                    throw Exception("input_t::input_t -- corpusSize set more than once");
                json_read_int(E, _corpusSize);
                set_corpusSize = true;
            }
            else if (correctThreshold_s.compare(name) == 0)
            {
                if (set_correctThreshold)
                    throw Exception("input_t::input_t -- correctThreshold set more than once");
                json_read_int(E, _correctThreshold);
                set_correctThreshold = true;
            }
            else if (randomBytes_s.compare(name) == 0)
            {
                if (set_randomBytes)
                    throw Exception("input_t::input_t -- salt set more than once");
                json_read_bytes_ex(E, _randomBytes);
                set_randomBytes = true;
            }
            else
                throw Exception("input_t::input_t -- unrecognized key value");
        }
    }
    catch (...)
    {
        free(root);
        throw;
    }
    free(root);
    if (!set_setSize)
        throw Exception("setSize is not set");
    if (!set_correctThreshold)
        throw Exception("correctThreshold is not set");
    if (!set_corpusSize)
        throw Exception("corpusSize is not set");
}
