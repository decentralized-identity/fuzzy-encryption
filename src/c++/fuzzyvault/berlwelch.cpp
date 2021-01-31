/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#include <ostream>
#include <iomanip>
#include "types.h"
#include "imod.h"
#include "matrix.h"
#include "berlwelch.h"
#include "exceptions.h"
#include "fuzzy.h"

poly_t berlekamp_welch(
    const std::vector<int>& as,
    const std::vector<int>&bs,
    const int k,
    const int t
    )
{
    if (as.size() != bs.size() || as.size() == 0)
        throw Exception("berlekamp_welch: |as| != |bs|");
    if (k <= 0 || t <= 0)
        throw Exception("berlekamp_welch: k <= 0 || t <= 0");
    const int n = (int)as.size();
    matrix_t m(n, n);
    matrix_t y(n, 1);
    
    for (int i = 0; i < n; i++)
    {
        const imod_t b = bs[i];
        std::vector<imod_t>apowers(k + t);
        imod_t::get_powers(as[i], apowers);
        for (int j = 0; j < k + t; j++)
            m.set(i, j, apowers[j]);
        for (int j = 0; j < t; j++)
            m.set(i, j + k + t, - (b * apowers[j]));
        y.set(i, 0, b * apowers[t]);
    }
    matrix_t x = m.solve(y);

    std::vector<imod_t> Qs(k + t);
    for (int i = 0; i < k + t; i++)
        Qs[i] = x.get(i, 0);
    poly_t Q(Qs);

    const int e = n - k - t;
    std::vector<imod_t> Es(e + 1);
    for (int i = 0; i < e; i++)
        Es[i] = x.get(k + t + i, 0);
    Es[e] = 1;
    poly_t E(Es);

    poly_t q;
    poly_t r;
    poly_t::div_rem(Q, E, q, r);
    if (r.degree() >= 0)
        throw fuzzy_vault::NoSolutionException();
    return q;
}
