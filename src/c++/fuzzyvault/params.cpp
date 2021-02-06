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
#include <iomanip>
#include <string>
#include "types.h"
#include "params.h"
#include "imod.h"
#include "crypto.h"
#include "parsing.h"
#include "exceptions.h"

params_t::params_t(int setSize,
                   int correctThreshold,
                   int corpusSize,
                   int prime,
                   std::vector<uint8_t>* randomBytes) :
    _setSize(setSize),
    _correctThreshold(correctThreshold),
    _corpusSize(corpusSize),
    _prime(prime)
{
    if (_setSize <= 0)
        throw Exception("params_t::params_t -- set_size <= 0");
    if (2 * _correctThreshold < _setSize)
        throw Exception("params_t::params_t -- 2 * correctThreshold < set_size");
    if (_setSize >= _corpusSize)
        throw Exception("params_t::params_t -- set_size >= corpusSize");
    if (_correctThreshold > _setSize)
        throw Exception("params_t::params_t -- correctThreshold > set_size");
    _salt.resize(32);

    crypto::rng_t rng(randomBytes);
    crypto::random_bytes(&rng, _salt);
    // std::vector<int> temp(_prime);
    // crypto::get_randomized_range(&rng, temp);
    std::vector<int> temp = crypto::rand_select(&rng, _prime, _setSize);
    _extractor.resize(_setSize);
    for (int i = 0; i < _setSize; i++)
        _extractor[i] = temp[i];
    temp.clear();
}

params_t::~params_t()
{
    clear();
}

void params_t::clear()
{
    _extractor.clear();
    _salt.clear();
}

params_t::params_t(std::string json_string)
{
    clear();
    const std::string setSize_s("setSize");
    const std::string corpusSize_s("corpusSize");
    const std::string correctThreshold_s("correctThreshold");
    const std::string extractor_s("extractor");
    const std::string prime_s("prime");
    const std::string salt_s("salt");

    // These flags are set when the value is read
    bool b_setSize = false;
    bool b_corpusSize = false;
    bool b_correctThreshold = false;
    bool b_prime = false;
    bool b_extractor = false;
    bool b_salt = false;

    struct json_value_s* root = json_parse(json_string.c_str(), json_string.length());

    if (root == 0)
        throw Exception("params_t::params_t -- json_parse failed");
    if (root->type != json_type_object)
        throw Exception("params_t::params_t -- json does not represent an object");
    try
    {
        json_object_s* object = (json_object_s*)root->payload;
        if (object == 0)
            throw Exception("params_t::params_t -- root->payload is null");
        json_object_element_s* E = object->start;
        for (size_t i = 0; i < object->length; i++, E = E->next) 
        {
            const char* name = E->name->string;
            if (name == 0)
                throw Exception("params_t::params_t -- name == 0");
            if (setSize_s.compare(name) == 0)
            {
                if (b_setSize)
                    throw Exception("params_t::params_t -- setSize set more than once");
                json_read_int(E, _setSize);
                b_setSize = true;
            }
            else if (corpusSize_s.compare(name) == 0)
            {
                if (b_corpusSize)
                    throw Exception("params_t::params_t -- corpusSize set more than once");
                json_read_int(E, _corpusSize);
                b_corpusSize = true;
            }
            else if (prime_s.compare(name) == 0)
            {
                if (b_prime)
                    throw Exception("params_t::params_t -- prime set more than once");
                json_read_int(E, _prime);
                b_prime = true;
            }
            else if (correctThreshold_s.compare(name) == 0)
            {
                if (b_correctThreshold)
                    throw Exception("params_t::params_t -- correctThreshold set more than once");
                json_read_int(E, _correctThreshold);
                b_correctThreshold = true;
            }
            else if (extractor_s.compare(name) == 0)
            {
                if (b_extractor)
                    throw Exception("params_t::params_t -- extractor set more than once");
                json_read_ints(E, _extractor);
                b_extractor = true;
            }
            else if (salt_s.compare(name) == 0)
            {
                if (b_salt)
                    throw Exception("params_t::params_t -- salt set more than once");
                json_read_bytes(E, _salt);
                b_salt = true;
            }
            else
            {
                throw Exception("params_t::params_t -- bad key value");
            }
        }
    }
    catch (...)
    {
        free(root);
        throw;
    }
    free(root);

    if (!b_setSize)
        throw Exception("params_t::params_t -- setSize not set");
    if (!b_corpusSize)
        throw Exception("params_t::params_t -- corpusSize not set");
    if (!b_correctThreshold)
        throw Exception("params_t::params_t -- correctThreshold not set");
    if (!b_prime)
        throw Exception("params_t::params_t -- prime not set");
    if (!b_extractor)
        throw Exception("params_t::params_t -- extractor not set");
    if (!b_salt)
        throw Exception("params_t::params_t -- salt not set");
}

std::ostream& operator<<(std::ostream& os, const params_t& params)
{
    os  << "{" << std::endl
        << "  \"setSize\": " << std::dec << params._setSize << "," << std::endl
        << "  \"corpusSize\": " << std::dec << params._corpusSize << "," << std::endl
        << "  \"correctThreshold\": " << std::dec << params._correctThreshold << "," << std::endl
        << "  \"prime\": " << std::dec << params._prime << "," << std::endl
        << "  \"extractor\": " << std::dec << params._extractor << "," << std::endl
        << "  \"salt\": \"" << params._salt << "\"" << std::endl
        << "}";
    return os;
}
