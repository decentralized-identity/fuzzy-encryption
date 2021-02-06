"""
    Utilities for the uses of the Fuzzy Key Recovery scheme
"""

from math import log, exp, pi

# pylint: disable=invalid-name

def logfac1(z: float) -> float:
    '''
    return an approximation to log_e((z-1)!)
    '''
    x: float = 1 / z
    y: float = x * x
    w: float = log(z)
    ans: float = z * (w - 1) + 0.5 * (log(2 * pi) - w)
    return ans + (x/12)*(1 - (y/30)*(1 - (2*y/7)*(1-(3*y/4))))

def logfac(z: float) -> float:
    'return an estimate of log_e(z!)'
    return logfac1(z + 1)

def logbinom(n: float, m: float) -> float:
    '''
    return an extimate of log_e(binomial(n, m))
    '''
    return logfac(n) - logfac(m) - logfac(n-m)

def logprob(N: int, n: int, m: int) -> float:
    '''
    log base e of the probablity of randomly drawing
    m green balls on n draws from an urn containing
    n green balls and N-n red balls
    '''
    return logbinom(N - n, n - m) + logbinom(n, m) - logbinom(N, n)

def log2prob(N: int, n: int, m: int) -> float:
    '''
    Given corpus size of "N" and a recovery word
    set size of "n" (same count as the original word set),
    Return the log base 2 of the probatilty of
    getting exactly "m" of the guesses right.
    '''
    return logprob(N, n, m) / log(2)

def entropy(N: int, n: int, m: int) -> float:
    'returns the entropy of guessing m out of n on n draws from N'
    lg2prb = log2prob(N, n, m)
    prb = exp(lg2prb / log(2))
    return - prb * lg2prb

def ScryptMemRequired(N=16384, r=8, p=1) -> int:
    '''
    Returns the memory required to compute the Scrypt key

    N := iterations count e.g. 16384 or 2048 or ...
    r := block size (affect memory and CPU), e.g. 8
    p := parallelism factor (# threads run in parallel), e.g. 1

    https://wizardforcel.gitbooks.io/practical-cryptography-for-developers-book/content/mac-and-key-derivation/scrypt.html
    '''
    return 128 * N * r * p
