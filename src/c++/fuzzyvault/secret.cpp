/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#include "secret.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <set>
#include "types.h"
#include "params.h"
#include "imod.h"
#include "crypto.h"
#include "json.h"
#include "poly.h"
#include "secret.h"
#include "matrix.h"
#include "utils.h"
#include "berlwelch.h"
#include "exceptions.h"
#include "fuzzy.h"
#include "parsing.h"

void secret_t::get_keys(const std::vector<int> words, 
                        const int count, 
                        std::vector<std::vector<uint8_t>>& keys
                       ) const
{
    if (keys.size() != 0)
        throw Exception("secret_t::get_keys -- keys is not empty");
    std::vector<uint8_t> ek;
    get_ek(words, ek);
    keys.resize(count);
    for (int i = 0; i < count; i++)
    {
        std::vector<uint8_t>& key = keys[i];
        get_key(ek, i, key);
    }
}

secret_t::secret_t(const params_t& params,
                   const std::vector<int> words
                  ) : _setSize(params._setSize),
                      _correctThreshold(params._correctThreshold),
                      _corpusSize(params._corpusSize),
                      _prime(params._prime)
{
    check_words(words, _setSize, _corpusSize);
    std::vector<int> sorted_words(words);
    std::sort(sorted_words.begin(), sorted_words.end());
    _extractor.assign(params._extractor.begin(), params._extractor.end());
    _salt.assign(params._salt.begin(), params._salt.end());
    gen_sketch(sorted_words, errorThreshold(), _sketch);
    get_hash(sorted_words, _hash);
}

void secret_t::get_scrypt(const std::string& prefix,
                          const std::vector<int>& words,
                          std::vector<uint8_t>& out
                         ) const
{
    out.clear();
    std::vector<uint8_t> pass(prefix.begin(), prefix.end());
    for (int word : words)
        pushback_int(word, pass);
    crypto::scrypt(pass, _salt, out);
}

void secret_t::get_hash(const std::vector<int>& words,
                        std::vector<uint8_t>& out
                       ) const
{
    std::string prefix = "original_words:";
    return get_scrypt(prefix, words, out);
}

secret_t::~secret_t()
{
    clear();
}

void secret_t::clear()
{
    _setSize = 0;
    _corpusSize = 0;
    _correctThreshold = 0;
    _prime = 0;
    _extractor.clear();
    _salt.clear();
    _sketch.clear();
    _hash.clear();
}

secret_t::secret_t(std::string json)
{
    clear();
    const std::string setSize_s("setSize");
    const std::string corpusSize_s("corpusSize");
    const std::string correctThreshold_s("correctThreshold");
    const std::string prime_s("prime");
    const std::string extractor_s("extractor");
    const std::string salt_s("salt");
    const std::string sketch_s("sketch");
    const std::string hash_s("hash");
    struct json_value_s* root = json_parse(json.c_str(), json.length());
    if (root == 0)
        throw Exception("secret_t:secret_t -- json_parse failed");
    try
    {
        json_object_s* object = (json_object_s*)root->payload;
        if (object == 0)
            throw Exception("secret_t::secret_t -- root->payload is null");
        json_object_element_s* E = object->start;
        for (size_t i = 0; i < object->length; i++, E = E->next) 
        {
            const char* name = E->name->string;
            if (name == 0)
                throw Exception("secret_t::secret_t -- name == 0");
            if (setSize_s.compare(name) == 0)
                json_read_int(E, _setSize);
            else if (corpusSize_s.compare(name) == 0)
                json_read_int(E, _corpusSize);
            else if (correctThreshold_s.compare(name) == 0)
                json_read_int(E, _correctThreshold);
            else if (prime_s.compare(name) == 0)
                json_read_int(E, _prime);
            else if (extractor_s.compare(name) == 0)
                json_read_ints(E, _extractor);
            else if (salt_s.compare(name) == 0)
                json_read_bytes(E, _salt);
            else if (sketch_s.compare(name) == 0)
                json_read_ints(E, _sketch);
            else if (hash_s.compare(name) == 0)
                json_read_bytes(E, _hash);
        }
    }
    catch (...)
    {
        free(root);
        throw;
    }
    free(root);
}

void secret_t::recover(const std::vector<int>& recoveryWords, 
                       std::vector<int>& recoveredWords
                      ) const
{
    std::vector<uint8_t> rhash;
    std::vector<int> sorted_words(recoveryWords);
    std::sort(sorted_words.begin(), sorted_words.end());
    get_hash(sorted_words, rhash);
    if (rhash == _hash)
    {
        recoveredWords.assign(sorted_words.begin(), sorted_words.end());
        return;
    }
    recover_words(recoveryWords, _sketch, errorThreshold(), recoveredWords);
    get_hash(recoveredWords, rhash);
    if (rhash != _hash)
        throw fuzzy_vault::NoSolutionException();
}

void secret_t::get_ek(const std::vector<int>&words,
                      std::vector<uint8_t>& out
                     ) const
{
    std::vector<int> aList(words);
    std::sort(aList.begin(), aList.end());
    const std::vector<int>& sList = _extractor;
    imod_t e(1);
    for (int i = 0; i < _setSize; i++)
        e = e * (imod_t(aList[i]) * imod_t(sList[i]));
    std::vector<uint8_t> pass = { 'k', 'e', 'y', ':' };
    pushback_int(e._n, pass);
    crypto::scrypt(pass, _salt, out);
}

void secret_t::get_key(const std::vector<uint8_t>& ek,
                       int count,
                       std::vector<uint8_t>& key
                      ) const
{
    key.clear();
    std::vector<uint8_t> pass;
    pushback_int(count, pass);
    crypto::hmac(pass, ek, key);
}

void secret_t::check_words(const std::vector<int>& words,
                           const int set_size,
                           const int corpus_size
                          )
{
    if (words.size() != static_cast<size_t>(set_size))
        throw Exception("check_words: words.size() != set_size");
    if (!utils::are_unique(words))
        throw Exception("check_words: words are not unique");
    std::set<int> word_set;
    for (int word : words)
    {
        if (word_set.find(word) != word_set.end())
            throw Exception("check_words: repeated words");
        if (!(0 <= word && word < corpus_size))
            throw Exception("check_words: !(0 <= word && word < corpus_size)");
        word_set.insert(word);
    }
}

void secret_t::gen_sketch(const std::vector<int>& words,
                          const int threshold,
                          std::vector<int>& sketch
                         )
{
    if (threshold <= 0 || words.size() <= static_cast<size_t>(threshold))
        throw Exception("gen_sketch: bad threshold");
    sketch.resize(threshold);
    poly_t poly = poly_t::from_roots(words);
    const int offset = (int)words.size() - threshold;
    for (int i = 0; i < threshold; i ++)
        sketch[i] = poly._coeffs[i + offset]._n;
}

poly_t secret_t::get_phigh(const std::vector<int>& ts,
                           const int s
                          )
{
    poly_t poly;
    poly.clear();
    const int offset = s - ts.size();
    for (size_t i = 0; i < ts.size(); i++)
        poly._coeffs[i + offset] = ts[i];
    poly._coeffs[offset + ts.size()] = 1;
    return poly;
}

void secret_t::recover_words(const std::vector<int>& words,
                             const std::vector<int>& sketch,
                             const int t,
                             std::vector<int>& out
                             )
{
    if (t % 2 != 0)
        throw Exception("recover_words -- t is not even");
    const int n = words.size();
    poly_t p_high = get_phigh(sketch, n);
    const std::vector<int>& a_coeffs = words;
    std::vector<int> b_coeffs(n);
    for (int i = 0; i < n; i++)
        b_coeffs[i] = p_high(a_coeffs[i]);
    poly_t p_low = berlekamp_welch(a_coeffs, b_coeffs, n - t, t / 2);
    poly_t p_diff = p_high - p_low;
    std::vector<root_t> roots;
    p_diff.find_roots(roots);
    if (roots.size() != static_cast<size_t>(n))
        throw fuzzy_vault::NoSolutionException();
    for (root_t r : roots)
    {
        if (r._count > 1)
            throw fuzzy_vault::NoSolutionException();
    }
    out.resize(n);
    for (int i = 0; i < n; i++)
        out[i] = roots[i]._root;
}

std::ostream& operator<<(std::ostream& os,
                         const secret_t& secret
                        )
{
    os  << "{" << std::endl
        << "  \"setSize\": " << std::dec << secret._setSize << "," << std::endl
        << "  \"corpusSize\": " << std::dec << secret._corpusSize << "," << std::endl
        << "  \"correctThreshold\": " << std::dec << secret._correctThreshold << "," << std::endl
        << "  \"prime\": " << std::dec << secret._prime << "," << std::endl
        << "  \"extractor\": " << std::dec << secret._extractor << "," << std::endl
        << "  \"salt\": \"" << secret._salt << "\"," << std::endl
        << "  \"sketch\": " << std::dec << secret._sketch << "," << std::endl
        << "  \"hash\": \"" << secret._hash << "\"" << std::endl
        << "}";
    return os;
}
