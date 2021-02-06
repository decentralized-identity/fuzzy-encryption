/*
* Copyright 2021 The Decentralized Identity Foundation
* Project Authors. All Rights Reserved.
*
* Licensed under the Apache License 2.0 (the "License"). You may not use
* this file except in compliance with the License. You can obtain a copy
* in the file LICENSE in the source distribution or at
* https://identity.foundation/
*/

#ifndef _MATRIX_H_
#define _MATRIX_H_

#include "types.h"
#include "imod.h"
#include <vector>

/// An exception internal to the matrix_t class. It is used
/// to indicate that the search of a pivot failed.
class GetPivotRowException {};

/// This is the representation of a matrix of modular values
struct matrix_t {
    const int _nRows;            ///< number of rows
    const int _nCols;            ///< number of columns
    std::vector<imod_t> _buf;    ///< a one-dimensional buffer containing all the values

    /// construct a matrix with all values set to zero
    /// @param nrows number of rows
    /// @param ncols number of columns
    matrix_t(int nrows, 
             int ncols
             );

    /// construct a matrix using the specfied values
    /// @param rows number of rows
    /// @param ncols number of cols
    /// @param values a one-dimensional buffer that supplies the values.
    ///     The size of the buffer must be equal to nrows * ncols.
    ///     The values are in row-column order, that is the values
    ///     in the rows are in consecutive positions starting with
    ///     the first row.
    matrix_t(int nrows,
             int ncols,
             const std::vector<int>& values
             );

    /// Checks that the row index is valid for this matrix.
    /// If it is found to be invalid an exception is thrown.
    /// @param row row index to be checked
    /// @returns void
    void validate_row(int row) const;

    /// Checks that the column index is valid for this matrix.
    /// If it is found to be invalid an exception is thrown.
    /// @param col column index to be checked
    /// @returns void
    void validate_col(int col) const;

    /// Gets the value at the specified position
    /// @param row row index
    /// @param col column index
    /// @returns the (modular) value at that position
    imod_t get(int row, int col) const;

    /// sets the value at the specified position
    /// @param row row index
    /// @param col column index
    /// @param v the value to be set in the matrix
    /// @returns the (modular) value at that position
    void set(int row, int col, const imod_t v);

    /// Gets the offset into the buffer corrsponding
    /// to the specified row and column indices
    /// @param row row index
    /// @param col column index
    /// @returns a legal offset into buf
    int get_offset(int row, int col) const;

    /// Find the column index of the first non-zero value in the specified row
    /// @param row row index
    /// @returns column index
    int find_pivot_column(int row) const;

    /// Returns the number of all zero rows at the bottom of this matrix after
    /// it has been put in echelon form
    /// @returns zero row count
    int count_null_rows() const;

    /// returns the transpose of this matrix
    /// @returns the transpose of this matrix
    matrix_t transpose() const;

    /// returns true if this matrix is singular. The matrix must be in
    /// echelon form before this can be called
    /// @returns true if singular
    bool is_singular() const;

    /// Appends a column to this matrix for the purpose of solving
    /// a set of equations
    /// @param rhs a column of values to be appended. rhs must
    ///     have one column and the same number of rows as this matrix.
    /// @returns an augmented version of this matrix
    matrix_t augment(const matrix_t& rhs) const;

    /// Swap the rows of this matrix in place
    /// @param row1 first row index
    /// @param row2 second row index
    /// @returns void
    void swap_rows(int row1, int row2);

    /// Finds the pivot row. This is a technical thing used when
    /// converting a matrix to echelon form. Consult any
    /// reference on Gaussian elimination.
    /// @param h a magic index
    /// @param k another magic index
    /// @return the index of the row to pivot
    int get_pivot_row(int h, int k) const;

    /// Pivots. This is a matrix thing. For details consult
    /// a reference on Gaussian elimination.
    /// @param h magic index
    /// @param k another magic index
    /// @returns void
    void pivot(int h, int k);

    /// Convert this matrix into upper echelon form with
    /// ones on the diagonal (if possible)
    void echelon();

    /// This is one of the processes in Gaussian elimination
    /// where we attempt to massage the matrix so that there
    /// are ones along the diagonal, zeros everywhere else
    /// except for the last column. See any reference
    /// on Gaussian elimination.
    void back_substitute();

    /// Solve the equations for a non-singular matrix.
    /// @returns an NRows X 1 matrix containing the solution
    matrix_t solve_normal_case();

    /// Attempt to solve an a singular set of equations.
    /// If the equations are consitent but not independed
    /// we will return a particular solution. If the
    /// equations are not consistent then we throw a NoSolution
    /// exception
    /// @returns an NRows X 1 matrix containing the solution
    matrix_t solve_singular_case();

    /// Find the first column of the row where the value is
    /// one. It is assumed that all preceeding values
    /// on the row are zero.
    /// @param row the row to be examined
    /// @returns column index of the first row who's value is one.
    int find_leading_one(int row) const;

    /// Returns the matrix X such that M * X = B where
    /// M is this matrix. B is a one column matrix with
    /// same number of rows as this matrix. The solution
    /// X has one column and the same number of rows
    /// as this matrix. If a solution cannot be found
    /// a NoSolution exception is thrown.
    /// @param B the source column matrix
    /// @returns the solution column matrix
    matrix_t solve(const matrix_t& B);

    /// Returns a particular solution in the form
    /// of an _nRows X 1 matrix
    ///
    /// This handles the case where the matrix
    /// is singular but there is a a solution.
    /// For example consider the set of two
    /// equations in two unknowns
    ///
    ///     x + y = 1
    ///    2x + 2y = 2
    ///
    /// The matrix equation is
    ///
    ///     [ 1  1 ][x] = [1]
    ///     [ 2  2 ][y] = [2]
    ///
    /// The determinate of the matrix is zero (singular),
    /// but there are an infinite number of solutions.
    /// One particular solution is (x, y) = (1, 0).
    ///
    /// Consider trying to solve this set of equations
    /// with gaussian elemination. The augmented matrix
    /// is
    ///
    ///     [ 1 1 1]
    ///     [ 2 2 2]
    ///
    /// When you convert this to upper echelon form
    /// you get
    ///
    ///     [ 1 1 1]
    ///     [ 0 0 0]
    ///
    /// The existence of lower rows of all zeros indicates
    /// that there exists an infinite number of solutions.
    /// In since the last row is all zeros. I choose to set
    /// the last unknown (y) to zero. This effectively eleminates
    /// the last row and the second to last column of the 
    /// augmented matrix giving an equation in x only
    ///
    ///     [1 1]
    ///
    /// Which means x = 1 is a solution (we have already set y = 0)
    /// This strategy can be extended to more than two variables.
    /// If you find rows at the bottom of an echelon form zero.
    /// Set the last variable to zero and eliminate the corresponding
    /// row and column. Try again with the reduced augmented matrix.
    /// You will eventually get a set of solvable equations which
    /// give the values of the variable that you have not already
    /// set to zero.
    ///
    /// @param null_count number of rows at the bottom that are all zeros
    /// @returns a particular solution
    matrix_t solve_solvable_singular(int null_count);
};

const matrix_t operator*(const matrix_t&a, const matrix_t& b);
std::ostream& operator<<(std::ostream& os, const matrix_t& m);

#endif
