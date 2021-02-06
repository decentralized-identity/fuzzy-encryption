/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#ifndef _IMOD_H_
#define _IMOD_H_

#include <iostream>
#include <vector>

/// A structure containing a single integer, a modular value
struct imod_t {
    static int _modulus;    ///< This is the global modulus for all numbers
    static int* _inverses;  ///< This is an pre-computed array of inverses of the modular numbers

    int _n; ///< The value of the modular number it must be in the range 0 .. _modulus - 1 */

    /// Verifies that the value of _n is reasonable
    void verify();

    /// constructor with no value
    imod_t() { _n = 0; }

    /// constructor with an arbitrary integer value. The input value is
    /// translated into the modular equivalent and placed in _n
    imod_t(int n);

    /// constructing from an integer
    const imod_t operator=(int i);

    /// Returns the modular inverse of this
    const imod_t inv() const;

    /// Allow use of '-' in the code
    imod_t operator-() const;

    /// Allows a cast to an integer
    operator int() const { return _n; }
    
    /// Establishes the value of the modulus for all modular numbers
    /// in the process. This must be done at the beginning. The
    /// global array _inverses is allocaed and filled, so
    /// cleanup() must be called at the end of the process.
    ///
    /// @param modulus The value of the global modulus
    /// @returns void
    static void initialize(int modulus);

    /// Cleanup all global resources allocated to support 
    /// modular arithmetic. This must be called at
    /// the end of the process.
    ///
    /// @returns void
    static void cleanup();

    /// Verifies that the specified integers is a legitimate 
    /// modular value. If not valid an exception is thrown
    ///
    /// @param n The integer to be verified
    static void verify(int n);

    /// Fills a list with powers of a given integer
    /// [1, a, a^2, ..., a^n]. The list is filled
    /// to the end of the given output list
    ///
    /// @param a The value to be used
    /// @param out A reference to a list of modular values
    ///     who's values will be set to increasing powers'
    ///     of a
    /// @returns void
    static void get_powers(int a, std::vector<imod_t>& out);
};

imod_t operator+(const imod_t a, const imod_t b);
imod_t operator-(const imod_t a, const imod_t b);
imod_t operator*(const imod_t a, const imod_t b);
imod_t operator/(const imod_t a, const imod_t b);
bool operator!=(const imod_t a, imod_t b);
bool operator==(const imod_t a, int b);
bool operator==(const imod_t a, const imod_t b);
bool operator!=(const imod_t a, int b);

std::ostream& operator<<(std::ostream& os, const imod_t imod);



#endif
