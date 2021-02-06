/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#ifndef _POLY_H_
#define _POLY_H_

#include <ostream>
#include <vector>
#include "imod.h"
#include "types.h"


/// Represents the root of a modular polynomial
///
/// The modularity of the values has been previously set by a
/// call to imod_t::initialize()

struct root_t {
    imod_t _root;    ///< value of the root
    int _count;      ///< multiplicity of the root
};

/// Represents a modular polynomial
///
/// The modularity of the polynomial has been fixed by a previous
/// call to imod_t::initialize().
///
/// In this incarnation I restrict the maximum number of coefficients
/// to 32. This means we can represent polynomials up to degree 31.
/// This structure carries an fixed size array of modular numbers.
/// In this way everyting remains on the stack. There is no heap
/// allocation. I reserve the right to change my mind in the future.
struct poly_t {
    static const int _coeff_count = 32;   ///< maximum number of coefficients 
    imod_t _coeffs[_coeff_count];          ///< fixed array to hold the coefficients

    /// constructs a polynomial of degree -1. All coefficients are zero.
    poly_t();                           
    
    /// constructs a polynomial using the specified integer coefficients.
    /// @param coeffs values to be converted to modular coefficients
    ///     The number of coefficients must be less than or equal to 32
    poly_t(const std::vector<int>& coeffs);

    /// constructs a polynomial using the specified modular coefficients.
    /// @param coeffs modular values to be used as coefficients
    ///     The number of coefficients must be less than or equal to 32
    poly_t(const std::vector<imod_t>& coeffs);

    /// Returns the degree of the polynomial
    int degree() const;

    /// Divides two polynomials
    /// @param a numerator
    /// @param b divisor
    /// @param q quotient
    /// @param r remainder
    static void div_rem(const poly_t& a,
                        const poly_t& b,
                        poly_t& q,
                        poly_t& r
                        );

    /// Evaluate a polynomial with the given argument
    /// @param x The argument
    /// @returns The value of the polynomial evaluated at x
    ///
    ///     std::vector<int> coeffs = { 1, 2, 3 };
    ///     poly_t poly(coeffs);
    ///     imod_t y = poly(17);    /* 17 is automatically cast to an imod_t */
    imod_t operator()(imod_t x);

    /// Fills in a vector of root_t structs with
    /// the value and multiplicity of the roots
    /// of the polynomial
    /// @param roots The destination
    void find_roots(std::vector<root_t>& roots);

    /// Sets the degree of the polynomial to -1
    /// all coefficients are zero.
    void clear();


    /// Creates a polynomial with the specified roots
    /// @param roots The roots of the polynomial
    static poly_t from_roots(const std::vector<int>& roots);

    /// Sample code
    static void test();
};

/// Subtracts two polynomials
/// @param a left hand side
/// @param b right hand side
/// @returns a - b
const poly_t operator-(const poly_t& a,
                       const poly_t& b
                       );

/// Adds two polynomials
/// @param a left hand side
/// @param b right hand side
/// @returns a + b
const poly_t operator+(const poly_t& a,
                       const poly_t& b
                       );

/// Multiplies two polynomials
/// @param a left hand side
/// @param b right hand side
/// @returns a * b
const poly_t operator*(const poly_t& a,
                       const poly_t& b
                       );

/// String representation of a polynomial to stream
std::ostream& operator<<(std::ostream& os, const poly_t& a);


#endif