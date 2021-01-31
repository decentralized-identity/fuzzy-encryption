/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#ifndef _BERLWELCH_H_
#define _BERLWELCH_H_

#include <vector>
#include "poly.h"

/// This is the Berlekamp-Welsch-Decoder as described in the whitepaper
/// 
/// @param as The recovery words as represented as indexes into the corpus
/// @param bs These are the results of applying p_high to each of the recovery words
/// @param k an integer equal to setSize (len(as)) minus the errorThreshold (t)
/// @param t errorTheshold (2 * (setSize - correctThreshold))
/// @return a polynomial p_low
poly_t berlekamp_welch(const std::vector<int>& as,
                       const std::vector<int>& bs,
                       int k,
                       int t
                       );
#endif
