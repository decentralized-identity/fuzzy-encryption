/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#include <memory.h>
#include "imod.h"
#include "poly.h"
#include "exceptions.h"


void poly_t::clear()
{
    memset(_coeffs, 0, sizeof(_coeffs));
}

poly_t::poly_t()
{
    clear();
}

poly_t::poly_t(const std::vector<int>& values)
{
    if (_coeff_count < values.size())
        throw Exception("CoeffCount < values.size()");
    clear();
    for (size_t i = 0; i < values.size(); i++)
        _coeffs[i] = imod_t(values[i]);
}

poly_t::poly_t(const std::vector<imod_t>& coeffs)
{
    if (_coeff_count < coeffs.size())
        throw Exception("CoeffCount < coeffs.size()");
    clear();
    for (size_t i = 0; i < coeffs.size(); i++)
        _coeffs[i] = coeffs[i];
}

int poly_t::degree() const
{
    int m = (int)(_coeff_count - 1);
    while (0 <= m && _coeffs[m] == 0)
        m -= 1;
    return m;
}

void poly_t::div_rem(
    const poly_t& numerator,
    const poly_t& denominator,
    poly_t& quotient,
    poly_t& remainder
)
{
    int m = numerator.degree();
    int n = denominator.degree();
    if (m == -1 || n == -1)
        throw Exception("m == -1 || n == -1");
    if (m < n)
        throw Exception("m < n");

    quotient.clear();
    remainder.clear();

    poly_t u = numerator;
    const poly_t& v = denominator;
    poly_t& q = quotient;
    poly_t& r = remainder;

    for (int k = m - n; k >= 0; k--)
    {
        q._coeffs[k] = u._coeffs[n + k] / v._coeffs[n];
        for (int j = n + k - 1; j >= k; j--)
            u._coeffs[j] = u._coeffs[j] - (q._coeffs[k] * v._coeffs[j - k]);
    }
    for (int i = 0; i < n; i++)
        r._coeffs[i] = u._coeffs[i];
}

const poly_t operator-(const poly_t& a, const poly_t& b)
{
    poly_t c;
    memcpy(c._coeffs, a._coeffs, sizeof(a._coeffs));
    const int n = b.degree() + 1;
    for (int i = 0; i < n; i++)
        c._coeffs[i] = a._coeffs[i] - b._coeffs[i];
    return c;
}

const poly_t operator+(const poly_t& a, const poly_t& b)
{
    poly_t c;
    memcpy(c._coeffs, a._coeffs, sizeof(a._coeffs));
    const int n = b.degree() + 1;
    for (int i = 0; i < n; i++)
        c._coeffs[i] = c._coeffs[i] + b._coeffs[i];
    return c;
}

const poly_t operator*(const poly_t& a, const poly_t& b)
{
    const int m = a.degree();
    const int n = b.degree();
    if (m < 0 || n < 0)
        throw Exception("m < 0 || n < 0");
    if (poly_t::_coeff_count <= m + n)
        throw Exception("CoeffCount <= m + n");
    poly_t ans;
    for (int k = 0; k <= m + n; k++)
    {
        for (int i = 0; i <= k; i++)
        {
            if (i > m || k - i > n)
                continue;
            ans._coeffs[k] = ans._coeffs[k] + (a._coeffs[i] * b._coeffs[k - i]);
        }
    }
    return ans;
}

imod_t poly_t::operator()(imod_t x)
{
    const int n = degree();
    imod_t ans = 0;
    for (int i = n; i >= 0; i--)
        ans = _coeffs[i] + (x * ans);
    return ans;
}

std::ostream& operator<<(std::ostream& os, const poly_t& poly)
{
    os << std::dec;
    const int deg = poly.degree();
    os << "[";
    for (int i = 0; i <= deg; i++)
        os << " " << poly._coeffs[i];
    os << " ]";
    return os;
}

void poly_t::find_roots(std::vector<root_t>& roots)
{
    for (int k = 0; k < imod_t::_modulus; k++)
    {
        imod_t x = k;
        if (0 != (*this)(x)._n)
            continue;
        bool found = false;
        for (root_t r : roots)
        {
            if (r._root == x)
            {
                r._count += 1;
                found = true;
                break;
            }
        }
        if (!found)
        {
            root_t r;
            r._root = x;
            r._count = 1;
            roots.push_back(r);
        }
    }
}

poly_t poly_t::from_roots(const std::vector<int>& roots)
{
    poly_t ans;
    ans.clear();
    ans._coeffs[0] = 1;
    for (int r : roots)
    {
        std::vector<int> coeffs = {-r, 1};
        poly_t t(coeffs);
        ans = ans * t;
    }
    return ans;
}

void poly_t::test()
{
    const int prime = 7001;
    std::cout << "prime <- " << prime << std::endl;
    imod_t::initialize(prime);
    try
    {
        std::vector<int> us = { 7, 12, 27, 27, 18 };
        std::vector<int> vs = { 1, 2, 3 };
        poly_t u(us);
        poly_t v(vs);
        poly_t q;
        poly_t r;
        poly_t::div_rem(u, v, q, r);
        poly_t x = (q * v) + r;

        std::cout << "u <- " << u << std::endl;
        std::cout << "v <- " << v << std::endl;
        std::cout << "q <- " << q << std::endl;
        std::cout << "r <- " << r << std::endl;
        std::cout << "x <- " << x << std::endl;
        std::cout << "(u - x) -> " << (u - x) << std::endl;

        std::vector<int> roots = {1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::cout << "roots <- " << roots << std::endl;
        poly_t poly = poly_t::from_roots(roots);
        std::cout << "poly <- " << poly << std::endl;
        std::vector<root_t> roots1;
        poly.find_roots(roots1);
        std::cout << "find_roots -> [";
        for (root_t X : roots1)
            std::cout << X._root << " ";
        std::cout << "]" << std::endl;
    }
    catch (const Exception& e)
    {
        imod_t::cleanup();
        throw;
    }
    catch(const std::exception& e)
    {
        imod_t::cleanup();
        throw;
    }
    imod_t::cleanup();
}