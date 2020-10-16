"""
Gaussian elimination for modular matrices

We take care to handle the case of singular matrices.
If it is possible to get a solution we will.
"""
from typing import Any, List
import click
# pylint: disable=no-name-in-module
from flint import nmod_mat, nmod                    # type: ignore
# pylint: enable=no-name-in-module

# pylint: disable=invalid-name, bad-whitespace

class NoSolutionError(Exception):
    """
    This is thrown to indicate that no solution exists
    """

class GetRowIndexError(Exception):
    """
    This exception is raised if get_row_index does
    finds that all the values within a given part
    of a column are all zero
    """

def get_row_index(A: nmod_mat, h: int, k: int) -> int:
    """
    Starting at row h and going down, find a
    row where where the k'th element is non-zero
    return the index of that row. If no row is found
    raise an IndexError exception
    """
    nrows = A.nrows()
    for i in range(h, nrows):
        if int(A[i, k]) != 0:
            return i
    raise GetRowIndexError

def swap_rows_to_get_nonzero_pivot_point(A: nmod_mat, h: int, k: int) -> None:
    """
    We want to make sure the pivot value A[h, k] is non-zero. To
    do this we will swap unprocessed rows so this is the case.
    If this cannot be done, an IndexError is raised.
    """
    swap_rows(A, h, get_row_index(A, h, k))

def row_echelon(A: nmod_mat) -> None:
    """
    Convert A in place to row echelon (upper triangular) form
    with ones along the diagonal (if possible)
    """
    nrows: int = A.nrows()
    ncols: int = A.ncols()
    h: int = 0   # pivot row
    k: int = 0   # pivot column
    while h < nrows and k < ncols:
        try:
            swap_rows_to_get_nonzero_pivot_point(A, h, k)
            assert A[h, k] != nmod(0, A.modulus())
            scale: nmod = 1 / A[h, k]
            for i in range(k, ncols):
                A[h, i] *= scale
            for i in range(h + 1, nrows):
                f: nmod = A[i, k]
                A[i, k] = 0
                for j in range(k + 1, ncols):
                    A[i, j] -= A[h, j] * f
            h += 1
            k += 1
        except GetRowIndexError:
            k += 1

def swap_rows(A: nmod_mat, i: int, j: int) -> None:
    """
    Swap rows i and j of the matrix A
    """
    if i == j:
        return
    for col in range(A.ncols()):
        temp = A[i, col]
        A[i, col] = A[j, col]
        A[j, col] = temp

def back_substitution(A: nmod_mat) -> None:
    """
    Assuming A is in row echelon form with A[i,i] == 1 (mod p)
    then perform back substitution
    """
    nrows = A.nrows()
    ncols = A.ncols()
    last = ncols - 1
    # leave the last row alone
    for row in range(nrows - 1, 0, -1):
        assert int(A[row, row]) == 1
        for row1 in range(row - 1, -1, -1):
            temp = A[row1, row] * A[row, last]
            A[row1, row] = 0
            A[row1, last] -= temp


def is_singular(A: nmod_mat) -> bool:
    """
    Checks to see if the system of equations is singular.
    A must be in row-echelon form.
    """
    for row in range(A.nrows()):
        if int(A[row, row]) == 0:
            return True
    return False

def count_null_rows(A: nmod_mat) -> int:
    """
    Starting from the bottom. Count the number
    of rows consisting of all zeros in the
    matrix A. A must be in row echelon form.
    """
    nrows = A.nrows()
    ncols = A.ncols()
    count = 0
    for row in range(nrows-1, -1, -1):
        for col in range(ncols):
            if int(A[row, col]) != 0:
                return count
        count += 1
    return count

def augment(M: nmod_mat, Y: nmod_mat) -> nmod_mat:
    """
    Create a matrix representing a set of linear
    equations by gluing on Y to the right side of M
    """
    nrows = M.nrows()
    ncols = M.ncols()
    if not(nrows > 0 and nrows == ncols):
        raise ValueError
    if not(Y.nrows() == nrows and Y.ncols() == 1):
        raise ValueError
    if not Y.modulus() == M.modulus():
        raise ValueError
    A = nmod_mat(nrows, ncols + 1, M.modulus())
    for row in range(nrows):
        for col in range(ncols):
            A[row, col] = M[row, col]
        A[row, ncols] = Y[row, 0]
    return A

def find_leading_one(A: nmod_mat, row: int) -> int:
    """
    Assuming that A is in echelon form where the
    leading value on a row is 1, find the zero-based
    column of the leading one for that row
    """
    for col in range(0, A.ncols()):
        if int(A[row, col]) == 1:
            return col
    raise NoSolutionError

def solve_singular_case(A: nmod_mat) -> nmod_mat:
    """
    Return a solution for a singular set of equations.
    The system may be singular because there are
    multiple solutions. In this case I will return
    a particular solution.
    If the solution does not exist raise a NoSolution
    exception.
    """
    nrows = A.nrows()
    ncols = A.ncols()
    null_count = count_null_rows(A)
    if null_count == 0:
        raise NoSolutionError()
    # a solution exists create a return value with all zeros
    X = nmod_mat(ncols-1, 1, A.modulus())
    # starting at the bottom, skip over the null rows
    for row in range(nrows-null_count-1, -1, -1):
        col = find_leading_one(A, row)
        # pick off the value of the solution from the last column
        X[col, 0] = A[row, ncols-1]
        # back substitute this row on the remaining rows
        for row1 in range(row-1, -1, -1):
            f = A[row1, col]
            A[row1, col] = 0
            for col1 in range(col+1, ncols):
                A[row1, col1] -= f * A[row, col1]
    return X

def solve_normal_case(A: nmod_mat) -> nmod_mat:
    """
    Completes the solution for the non-singular case
    by performing back substution on the matrix
    A which must be in echelon form comming in.
    """
    nrows = A.nrows()
    ncols = A.ncols()
    back_substitution(A)
    X = nmod_mat(nrows, 1, A.modulus())
    for i in range(nrows):
        X[i, 0] = A[i, ncols - 1]
    return X

def solve(m: nmod_mat, b: nmod_mat) -> nmod_mat:
    """
    Solves m * x = b for x

    Returns the solution b. Raises ValueError
    if no solution exists
    """
    a = augment(m, b)
    row_echelon(a)
    if is_singular(a):
        x = solve_singular_case(a)
    else:
        x = solve_normal_case(a)
    assert m * x == b
    return x

def create_nmod_mat(m: List[Any], p: int) -> nmod_mat:
    """
    Return the nmod_mat created from an integer matrix and prime
    This is called by test() (see below)
    """
    nrows = len(m)
    if isinstance(m[0], int):
        M = nmod_mat(nrows, 1, p)
        for row in range(nrows):
            M[row, 0] = m[row]
    elif isinstance(m[0], list):
        ncols = len(m[0])
        M = nmod_mat(nrows, ncols, p)
        for row in range(nrows):
            if len(m[row]) != ncols:
                raise ValueError
            for col in range(ncols):
                M[row, col] = m[row][col]
    else:
        raise ValueError
    return M

def work(case: int) -> None:
    """
    Demonstration test
    """
    if case == 0:
        P = 13
        m = [
            [ 1,  5, 12, 10, 11],
            [ 1,  7, 10,  2,  1],
            [ 1,  8, 12,  1,  8],
            [ 1,  9,  3, 12,  4],
            [ 1, 10,  9, 11,  6]
        ]
        y = [10, 6, 1, 3, 5]
    elif case == 1:
        P = 7
        m = [[1, 2, 3], [2, 4, 6], [1, 1, 1]]
        y = [4, 1, 1]
    elif case == 2:
        P = 13
        m = [
            [1,  0, 0,  0, 0],
            [1,  2, 4,  0, 0],
            [1,  3, 9,  0, 0],
            [1,  4, 3,  0, 0],
            [1, 10, 9, 11, 6]
        ]
        y= [0, 0, 0, 0, 5]
    elif case == 3:
        P = 13
        m = [
            [1, 1, 1, 1],
            [1, 1, 1, 1],
            [1, 1, 1, 1],
            [1, 1, 1, 2]
        ]
        y = [0, 0, 0, 3]
    else:
        print("Please specify a case!")
        return

    M = create_nmod_mat(m, P)
    Y = create_nmod_mat(y, P)
    print("\nM:")
    print(M)
    print("\nY:", Y.transpose())
    try:
        X = solve(M, Y)
        print("\nX:", X.transpose())
        print("\nM * X:", (M * X).transpose())
    except NoSolutionError:
        print("\nNo solution exists")


@click.command()
@click.option('--case', type=int, prompt='case', help="case number 0 .. 3")
def test(case: int) -> None:
    work(case)

if __name__ == '__main__':
    # pylint: disable=no-value-for-parameter
    # test()
    work(2)
