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
#include <memory.h>
#include <assert.h>
#include "imod.h"
#include "crypto.h"
#include "types.h"
#include "exceptions.h"

int imod_t::_modulus = 0;
int* imod_t::_inverses = 0;


imod_t imod_t::operator-() const
{
    return imod_t(- _n);
}

const imod_t imod_t::inv() const
{
    if (_n == 0)
        throw Exception("imod_t::inv zero division error");
    return imod_t(_inverses[_n]);
}

imod_t operator+(const imod_t a, const imod_t b)
{
    return imod_t(a._n + b._n);
}

imod_t operator-(const imod_t a, const imod_t b)
{
    return imod_t(a._n - b._n);
}

imod_t operator*(const imod_t a, const imod_t b)
{
    return imod_t(a._n * b._n);
}

imod_t operator/(const imod_t a, const imod_t b)
{
    return a * b.inv();
}

bool operator==(const imod_t a, const imod_t b)
{
    return a._n == b._n;
}

const imod_t imod_t::operator=(int i)
{
    imod_t::verify(i);
    _n = i;
    return *this;
}

bool operator!=(const imod_t a, int b)
{
    imod_t::verify(b);
    return a._n != b;
}

bool operator==(const imod_t a, int b)
{
    imod_t::verify(b);
    return a._n == b;
}

void imod_t::verify(int n)
{
    if (!(0 <= n && n < _modulus))
        throw Exception("bad modular value");
}

bool operator!=(const imod_t a, imod_t b)
{
    return a._n != b._n;
}

void imod_t::initialize(int modulus)
{
    assert(_modulus == 0);
    assert(_inverses == 0);
    if (modulus <= 0)
        throw Exception("invalid value for the modulus");
    if (!(crypto::is_prime(modulus)))
        throw Exception("modulus is not prime");
    if (modulus > 0x8000)
        throw Exception("modulus is too large");
    _inverses = new int[modulus];
    if (_inverses == 0)
        throw Exception("failed to allocae _inverses");
    _modulus = modulus;
    _inverses[0] = 0;
    for (int i = 1; i < _modulus; i++)
    {
        for (int j = i; j < _modulus; j++)
        {
            if (((i * j) % _modulus) == 1)
            {
                _inverses[i] = j;
                _inverses[j] = i;
            }
        }
    }
}

void imod_t::cleanup()
{
    if (_inverses != 0)
    {
        delete _inverses;
        _inverses = 0;
    }
    _modulus = 0;
}

imod_t::imod_t(int n)
{
    if (0 <= n and n < _modulus)
        _n = n;
    else if (n > 0)
        _n = n - ((n / _modulus) * _modulus);
    else
        _n = n + ((_modulus - n - 1)/ _modulus) * _modulus;
}

void imod_t::verify()
{
    if (!(0 <= _n && _n < _modulus))
        throw Exception("imod_t::verify failed");
}

std::ostream& operator<<(std::ostream& os, const imod_t imod)
{
    os << imod._n;
    return os;
}

void imod_t::get_powers(int a, std::vector<imod_t>& out)
{
    const imod_t x = imod_t(a);
    imod_t y = imod_t(1);
    for (size_t i = 0; i < out.size(); i++)
    {
        out[i] = y;
        y = y * x;
    }   
}
