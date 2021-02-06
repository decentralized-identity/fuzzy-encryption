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
#include "types.h"
#include "matrix.h"
#include "exceptions.h"
#include "fuzzy.h"

matrix_t::matrix_t(int nrows,
                   int ncols
                   ) : _nRows(nrows), _nCols(ncols)
{
    if (nrows <= 0 || ncols <= 0)
        throw Exception("matrix_t::matrix_t: invalid arguments");
    _buf.resize(nrows * ncols);
}

matrix_t::matrix_t(int nrows,
                   int ncols,
                   const std::vector<int>& values
                   ) : _nRows(nrows), _nCols(ncols)
{
    if (nrows <= 0 || ncols <= 0)
        throw Exception("matrix_t::matrix_t: invalid arguments");
    if (values.size() != static_cast<size_t>(nrows * ncols))
        throw Exception("matrix_t::matrix_t: invalid arguments");
    _buf.resize(nrows * ncols);

    int k = 0;
    for (int row = 0; row < _nRows; row++)
    {
        for (int col = 0; col < _nCols; col++)
        {
            set(row, col, values[k]);
            k += 1;
        }
    }
}

void matrix_t::validate_col(int col) const
{
    if (0 <= col && col < _nCols)
        return;
    throw Exception("col out of range");
}

void matrix_t::validate_row(int row) const
{
    if (0 <= row && row < _nRows)
        return;
    throw Exception("row out of range");
}

int matrix_t::get_offset(int row,
                         int col
                         ) const
{
    validate_row(row);
    validate_col(col);
    int ans = row * _nCols + col;
    return ans;
}

imod_t matrix_t::get(int row,
                     int col
                     ) const
{
    const int k = get_offset(row, col);
    return _buf[k];
}

void matrix_t::set(int row,
                   int col,
                   const imod_t v
                   )
{
    const int k = get_offset(row, col);
    _buf[k] = v;
}

int matrix_t::find_pivot_column(int row) const
{
    for (int col = 0; col < _nCols; col++)
    {
        if (get(row, col)._n == 1)
            return col;
    }
    throw fuzzy_vault::NoSolutionException();
}

int matrix_t::count_null_rows() const
{
    int count = 0;
    for (int row = _nRows - 1; row >= 0; row--)
    {
        for (int col = 0; col < _nCols; col++)
        {
            if (get(row, col) != 0)
                return count;
        }
        count += 1;
    }
    return count;
}

matrix_t matrix_t::transpose() const
{
    matrix_t ans(_nCols, _nRows);
    for (int row = 0; row < _nRows; row++)
    {
        for (int col = 0; col < _nCols; col++)
            ans.set(col, row, get(row, col));
    }
    return ans;
}

bool matrix_t::is_singular() const
{
    for (int row = 0; row < _nRows; row++)
    {
        if (get(row, row) == 0)
            return true;
    }
    return false;
}

matrix_t matrix_t::augment(const matrix_t& rhs) const
{
    if (_nRows != rhs._nRows)
        throw Exception("matrix_t::augment -- matrices are incompatible");
    matrix_t ans(_nRows, _nCols + rhs._nCols);
    for (int row = 0; row < _nRows; row++)
    {
        for (int col = 0; col < _nCols; col++)
            ans.set(row, col, get(row, col));
        for (int col = 0; col < rhs._nCols; col++)
            ans.set(row, col + _nCols, rhs.get(row, col));
    }
    return ans;
}

void matrix_t::swap_rows(int row1,
                         int row2
                         )
{
    validate_row(row1);
    validate_row(row2);
    if (row1 == row2)
        return;
    for (int col = 0; col < _nCols; col++)
    {
        const imod_t t = get(row1, col);
        set(row1, col, get(row2, col));
        set(row2, col, t);
    }
}

int matrix_t::get_pivot_row(int h, int k) const
{
    for (int i = h; i < _nRows; i++)
    {
        if (get(i, k) != 0)
            return i;
    }
    throw GetPivotRowException();
}

void matrix_t::pivot(int h,
                     int k
                     )
{
    swap_rows(h, get_pivot_row(h, k));
}

void matrix_t::echelon()
{
    int h = 0;
    int k = 0;
    while (h < _nRows && k < _nCols)
    {
        try
        {
            pivot(h, k);
            const imod_t scale = get(h, k).inv();
            for (int i = k; i < _nCols; i++)
                set(h, i, scale * get(h, i));
            for (int i = h + 1; i < _nRows; i++)
            {
                const imod_t f = get(i, k);
                set(i, k, 0);
                for (int j = k + 1; j < _nCols; j++)
                    set(i, j, get(i, j) - (get(h, j) * f));
            }
            h++;
            k++;
        }
        catch (GetPivotRowException)
        {
            k++;
        }
    }
}

const matrix_t operator*(const matrix_t&a,
                         const matrix_t& b
                         )
{
    if (a._nCols != b._nRows)
        throw Exception("matrices cannot be multiplied");
    matrix_t ans(a._nRows, b._nCols);
    for (int row = 0; row < ans._nRows; row++)
    {
        for (int col = 0; col < ans._nCols; col++)
        {
            imod_t x = 0;
            for (int k = 0; k < ans._nRows; k++)
                x = x + (a.get(row, k) * b.get(k, col));
            ans.set(row, col, x);
        }
    }
    return ans;
}

void matrix_t::back_substitute()
{
    const int last = _nCols - 1;
    for (int row = _nRows - 1; row > 0; row--)
    {
        for (int row1 = row - 1; row1 >= 0; row1--)
        {
            const imod_t temp = get(row1, row) * get(row, last);
            set(row1, row, 0);
            set(row1, last, get(row1, last) - temp);
        }
    }
}

matrix_t matrix_t::solve_normal_case()
{
    back_substitute();
    matrix_t X(_nRows, 1);
    for (int i = 0; i < _nRows; i++)
        X.set(i, 0, get(i, _nCols - 1));
    return X;
}

int matrix_t::find_leading_one(int row) const
{
    for (int col = 0; col < _nCols; col++)
    {
        if (get(row, col) == 1)
            return col;
    }
    throw fuzzy_vault::NoSolutionException();
}

matrix_t matrix_t::solve_singular_case()
{
    if (_nCols != _nRows + 1)
        throw Exception("Matrix not augmented correctly");
    int const null_count = count_null_rows();
    if (null_count == 0)
        throw fuzzy_vault::NoSolutionException();
    return solve_solvable_singular(null_count);
}

matrix_t matrix_t::solve_solvable_singular(int null_count)
{
    matrix_t X(_nRows, 1);
    for (int row = _nRows - null_count - 1; row >= 0; row--)
    {
        const int col = find_leading_one(row);
        X.set(col, 0, get(row, _nCols - 1));
        for (int row1 = row - 1; row1 >= 0; row1--)
        {
            const imod_t f = get(row1, col);
            set(row1, col, 0);
            for (int col1 = col + 1; col1 < _nCols; col1++)
                set(row1, col1, get(row1, col1) - (f * get(row, col1)));
        }
    }
    return X;
}

matrix_t matrix_t::solve(const matrix_t& B)
{
    matrix_t A = augment(B);
    A.echelon();
    if (A.is_singular())
        return A.solve_singular_case();
    else
        return A.solve_normal_case();
}

std::ostream& operator<<(std::ostream& os,
                         const matrix_t& mat
                         )
{
    int nrows = mat._nRows;
    int ncols = mat._nCols;

    os << std::dec << std::setfill(' ');
    if (nrows == 1)
    {
        os << '[';
        for (int col = 0; col < ncols; col++)
            os << " " << std::setw(4) << mat.get(0, col);
        os << ']';
        return os;
    }
    os << '[' << std::endl;
    for (int row = 0; row < nrows; row++)
    {
        for (int col = 0; col < ncols; col++)
            os << " " << std::setw(4) << mat.get(row, col);
        os << std::endl;
    }
    os << ']';
    return os;
}
