"""
fuzzyvault.py

This contains my attempt at creating a Python version
of pseudocode for the key recovery project.

There are three functions of interest:

    gen_params, gen_secret, gen_keys

"""

# pylint: disable=no-name-in-module, invalid-name

import json
import secrets
import binascii
from typing import List
import hashlib
import hmac
import scrypt                                       # type: ignore
import sympy                                        # type: ignore
from flint import nmod_poly, nmod_mat, nmod         # type: ignore
import gauss

class FuzzyError(Exception):
    """
    Exception class

    This is the exception that is thrown for all errors. I
    do not return errors. Errors result in exceptions. It
    is up to the caller of GenerateSecret and RecoverSecret
    to catch this exception in order to detect errors. For
    example, if RecoverSecret finds that that the recovery
    words are insufficient to recover the keys, an FuzzyError
    exception is thrown.
    """
    def __init__(self, message: str):
        Exception.__init__(self)
        self.message = message
    def __repr__(self) -> str:
        return self.message

class FuzzyInput:
    """
    represents the input to gen_params
    """
    def __init__(self, json_string: str):
        "constructor from json string representation"
        obj = json.loads(json_string)
        self.setSize: int = obj["setSize"]
        self.correctThreshold: int = obj["correctThreshold"]
        self.corpusSize: int = obj["corpusSize"]
        try:
            self.randomBytes = hex_list_to_bytes(obj["randomBytes"])
        except KeyError:
            self.randomBytes = None
    def get_object(self):
        "return a dictionary representing the input"
        ans = {
            "setSize" : self.setSize,
            "correctThreshold" : self.correctThreshold,
            "corpusSize": self.corpusSize
        }
        if self.randomBytes:
            ans["randomBytes"] = bytes_to_hex_list(self.randomBytes, 32)
        return ans
    def __repr__(self) -> str:
        "for printing"
        return json.dumps(self.get_object(), indent=2)

class Params:
    """
    represents the output of gen_params and the input to gen_secret
    """
    def __init__(self, input_json: str):
        "constructor"
        obj = json.loads(input_json)
        self.setSize: int = obj["setSize"]
        self.correctThreshold: int = obj["correctThreshold"]
        self.corpusSize: int = obj["corpusSize"]
        self.prime: int = obj["prime"]
        self.extractor: List[int] = obj["extractor"]
        self.salt: bytes = hex_to_bytes(obj["salt"])
    def get_object(self):
        "return a dictionary representing the input"
        return {
            "setSize" : self.setSize,
            "correctThreshold" : self.correctThreshold,
            "corpusSize": self.corpusSize,
            "prime": self.prime,
            "extractor": self.extractor,
            "salt": bytes_to_hex(self.salt)
        }
    def __repr__(self) -> str:
        "for printing"
        return json.dumps(self.get_object(), indent=2)

# pylint: disable=too-many-instance-attributes
class Secret:
    """
    represents the output gen_secret and the input to gen_keys
    """
    def __init__(self, json_string: str, words_json: str = None):
        """
        constructor

        There are two possibilities:
            1. The json_string represents a Params object and
               the words_json string is None
            2. The json_string represents a Secret object and
               the words_json string is None
        """
        obj = json.loads(json_string)
        self.setSize: int = obj["setSize"]
        self.correctThreshold: int = obj["correctThreshold"]
        self.corpusSize: int = obj["corpusSize"]
        self.prime: int = obj["prime"]
        self.extractor: List[int] = obj["extractor"]
        self.salt: bytes = hex_to_bytes(obj["salt"])
        if words_json:
            # json_string represents a Params object.
            # The sketch and hash must be calculated.
            words = json.loads(words_json)
            check_words(words, self.setSize, self.corpusSize)
            errorThreshold = 2 * (self.setSize - self.correctThreshold)
            self.sketch: List[int] = gen_sketch(words, self.prime, errorThreshold)
            words.sort()
            self.hash: bytes = scrypt.hash("original_words:" + str(words), self.salt)
        else:
            # this is for the case where the input is a JSON string
            # represents a Secret. In this case just copy the data.
            self.sketch: List[int] = obj["sketch"]
            self.hash: bytes = hex_to_bytes(obj["hash"])

    def get_object(self):
        "return a dictionary representing the input"
        return {
            "setSize" : self.setSize,
            "correctThreshold" : self.correctThreshold,
            "corpusSize": self.corpusSize,
            "prime": self.prime,
            "extractor": self.extractor,
            "salt": bytes_to_hex(self.salt),
            "sketch": self.sketch,
            "hash": bytes_to_hex(self.hash)
        }

    def __repr__(self) -> str:
        "for printing"
        return json.dumps(self.get_object(), indent=2)
# pylint: enable=too-many-instance-attributes

class Rng:
    """
    Random number generator class
    """
    def __init__(self, randomBytes: bytes):
        "constructor"
        self.randomBytes = randomBytes
        self.index = 0

    def get_bytes(self, count: int) -> bytes:
        "returns the required number of random bytes"
        if self.randomBytes:
            ans = self.randomBytes[self.index: self.index + count]
            self.index += count
            return ans
        return random_bytes(count)

    def get_int(self) -> int:
        "returns a random unsigned 32-bit integer"
        if self.randomBytes:
            ans = Rng.create_int(self.randomBytes[self.index: self.index + 4])
            self.index += 4
            return ans
        return Rng.create_int(random_bytes(4))

    def select(self, n, m) -> List[int]:
        """
        randomly selects m integers from 0 ... n
        """
        assert 0 < m <= n
        xs = list(range(n))
        for i in range(m):
            k = i + (self.get_int() % (n - i))
            t = xs[i]
            xs[i] = xs[k]
            xs[k] = t
        return xs[:m]

    @staticmethod
    def create_int(xs: bytes) -> int:
        """
        creates an unsigned 32-bit integer from 4 bytes
        in little endian order
        """
        return xs[0] + 256 * (xs[1] + 256 * (xs[2] + 256 * xs[3]))

def gen_params(input_json: str) -> str:
    """
    input: JSON string representing an Input class
    output: JSON string representing a Params class
    """
    obj = json.loads(input_json)
    fInput = FuzzyInput(input_json)
    rng = Rng(fInput.randomBytes)
    obj["prime"] = first_prime_greater_than(fInput.corpusSize)
    obj["salt"] = bytes_to_hex(rng.get_bytes(32))
    obj["extractor"] = rng.select(obj["prime"], fInput.setSize)
    return json.dumps(obj, indent=2)

def gen_secret(params_json: str, original_words_json: str) -> str:
    """
    input: output of gen_params, JSON string representing a list of integers (words)
    output: JSON string representing a Secret class
    """
    secret: Secret = Secret(params_json, original_words_json)
    return str(secret)

def get_hash(words: List[int], salt: bytes) -> bytes:
    """
    Returns the slow hash of a set of integers (words)
    """
    _words = words[:]
    _words.sort()
    return scrypt.hash("original_words:" + str(_words), salt)

def get_ek(secret: Secret, words: List[int]) -> bytes:
    """
    Returns the bytes constant used in the RecoverSecret loop
    It is passed into key_derivation

    returns  (s_1 * a_1) * (s_2 * a_2) * ... * (s_n * a_n) (mod p)
    """
    sList = secret.extractor
    aList = words
    aList.sort()
    e: nmod = nmod(1, secret.prime)
    for i in range(secret.setSize):
        e *= sList[i] * aList[i]
    return scrypt.hash("key:" + str(e), secret.salt)

def gen_keys(secret_json: str, recovery_words_json: str, key_count: int) -> str:
    """
    input: output of gen_secret, JSON string representing a list of integers (words)
           that are a guess of the original words passed into gen_secret

    output: a JSON string representing a list of keys where each key is
            represented as a hex string
    """
    assert key_count >= 0
    if key_count == 0:
        return '[]'
    secret = Secret(secret_json)
    words = json.loads(recovery_words_json)
    check_words(words, secret.setSize, secret.corpusSize)
    words.sort()
    if secret.hash == get_hash(words, secret.salt):
        ek = get_ek(secret, words)
    else:
        ek = get_ek(secret, words)
        recouped_words = recoup_words(secret, words)
        if secret.hash != get_hash(recouped_words, secret.salt):
            raise FuzzyError("Hashes do not match")
        ek = get_ek(secret, recouped_words)
    keys = [bytes_to_hex(key_derivation(ek, k)) for k in range(key_count)]
    return json.dumps(keys, indent=2)

def recoup_words(secret: Secret, words: List[int]) -> List[int]:
    """
    Recover the words using the recovery words as a guess
    """
    errorThreshold = 2 * (secret.setSize - secret.correctThreshold)
    if len(words) != secret.setSize:
        raise FuzzyError("length of words is not equal to setsize")
    p_high: nmod_poly = get_phigh(secret.sketch, secret.setSize, secret.prime)
    a_coeffs: List[int] = words
    b_coeffs: List[int] = [p_high(a) for a in a_coeffs]
    p_low: nmod_poly = \
        Berlekamp_Welch(
            a_coeffs,
            b_coeffs,
            secret.setSize - errorThreshold,
            errorThreshold // 2,
            secret.prime)
    p_diff: nmod_poly = p_high - p_low
    if has_repeated_roots(p_diff, secret.prime):
        raise FuzzyError("repeated roots have been detected")
    return find_roots(p_diff)

def isprime(candidate: int) -> bool:
    """
    checks if an integer is prime
    """
    return sympy.isprime(candidate)

def create_poly(xs: List[int], p: int) -> nmod_poly:
    """
    returns the polynomial (z - x[0]) ... (z - x[N-1]) mod p
    """
    ans: nmod_poly = nmod_poly([1], p)
    for x in xs:
        ans *= nmod_poly([-x, 1], p)
    return ans

def gen_sketch(words: List[int], prime: int, thresh: int) -> List[int]:
    """
    Return a list of t integers
    """
    if thresh % 2 != 0:
        raise FuzzyError("bad error threshold")
    poly: nmod_poly = create_poly(words, prime)
    nwords = len(words)
    return [int(x) for x in poly.coeffs()[nwords - thresh: nwords]]

def get_phigh(tlist: List[int], s: int, p: int) -> nmod_poly:
    """
    return a polynomial object where the caller specifies the highest coefficients
    except the highest coefficient which takes the value 1
    """
    nzeros: int = s - len(tlist)
    coeffs: List[int] = [0] * nzeros
    coeffs.extend(tlist)
    coeffs.append(1)
    return nmod_poly(coeffs, p)

def has_repeated_roots(poly: nmod_poly, prime: int) -> bool:
    """
    returns True if the polynomial has repeated roots
    """
    if prime < 2 or not isprime(prime):
        raise FuzzyError("prime is not prime")
    temp: List[int] = [0] * (prime + 1)
    temp[-1] = 1
    temp[1] = -1
    zpoly: nmod_poly = nmod_poly(temp, prime)
    result: nmod_poly = zpoly % poly
    return result.coeffs() != []

def mod_get_powers(a: int, n: int, p: int) -> List[nmod]:
    """
    returns a list 1, a, a^2, a^{n-1} mod p

    example:

    mod_get_powers(3, 4, 7) = [1, 3, 9, 27] (mod 7)  = [1, 3, 2, 6]
    """
    if n < 1:
        raise FuzzyError("upper power n is not positive")
    if p < 2 or not isprime(p):
        raise FuzzyError("prime is not prime")
    if a == 0:
        ans = [nmod(1, p)]
        ans.extend([nmod(0, p)] * (n-1))
        return ans

    def my_gen():
        """
        This is a generator that yields
        1, a, a^2, ..., a^(n-1) (mod p)
        """
        x = nmod(a, p)
        y = nmod(1, p)
        for _ in range(n):
            yield y
            y *= x
    return list(my_gen())

# pylint: disable=too-many-locals
def Berlekamp_Welch(
        aList: List[int],   # inputs
        bList: List[int],   # received codeword
        k: int,
        t: int,
        p: int              # prime
    ) -> nmod_poly:
    """
    Berlekamp-Welch-Decoder

    This function throws an exception if no solution exists

    see https://en.wikipedia.org/wiki/Berlekamp%E2%80%93Welch_algorithm
    """
    if len(aList) < 1:
        raise FuzzyError("aList is empty")
    if len(aList) != len(bList):
        raise FuzzyError("bList is empty")
    if k < 1 or t < 1:
        raise FuzzyError("k={0} and t={1} are not consistent".format(k, t))
    if p < 2 or not isprime(p):
        raise FuzzyError("p is not prime")

    n = len(aList)

    # Create the Berlekamp-Welch system of equations
    # and store them as an n x n matrix 'm' and a
    # constant source vector 'y' of length n

    m_entries: List[nmod] = []
    y_entries: List[nmod] = []
    for i in range(n):
        a: int = aList[i]
        b: int = bList[i]
        apowers: List[nmod] = mod_get_powers(a, k + t, p)
        for j in range(k+t):
            m_entries.append(apowers[j])
        for j in range(t):
            m_entries.append(-b * apowers[j])
        y_entries.append(b * apowers[t])

    m: nmod_mat = nmod_mat(n, n, m_entries, p)
    y: nmod_mat = nmod_mat(n, 1, y_entries, p)

    # solve the linear system of equations m * x = y for x

    try:
        x = gauss.solve(m, y).entries()
    except gauss.NoSolutionError:
        raise FuzzyError("No solution exists")

    # create the polynomials Q and E

    Qs: List[nmod] = x[:k+t]
    Es: List[nmod] = x[k+t:]
    Es.append(nmod(1, p))
    Q: nmod_poly = nmod_poly(Qs, p)
    E: nmod_poly = nmod_poly(Es, p)

    Answer: nmod_poly = Q // E
    Remainder: nmod_poly = Q - Answer * E
    if len(Remainder.coeffs()) > 0:
        raise FuzzyError("Remainder is not zero")
    return Answer
# pylint: enable=too-many-locals

def brute_force_find_roots(poly: nmod_poly) -> List[int]:
    """
    Find the roots of a polynomial by evaluating every
    possible argument.
    """
    return [x for x in range(poly.modulus()) if int(poly(x)) == 0]

def flint_find_roots(poly: nmod_poly) -> List[int]:
    """
    Find the roots of the square free polynomial
    using the flint.nmod_poly.factor method
    """
    _, roots = poly.factor()
    return [int(-f.coeffs()[0]) for f, _ in roots]

def find_roots(poly: nmod_poly) -> List[int]:
    """
    return roots of the polynomial

    you can use either brute_force_find_roots or
    flint_find_roots

    Called by recover_words
    """
    # return brute_force_find_roots(poly)
    return flint_find_roots(poly)

def key_derivation(ek: bytes, count: int) -> bytes:
    """
    Returns a 512-bit key made unique by the count parameter
    """
    secret: bytes = bytes(str(count), 'utf-8')
    mac: hmac.HMAC = hmac.new(secret, ek, digestmod=hashlib.sha512)
    return mac.digest()

def first_prime_greater_than(k: int) -> int:
    """
    return the first prime greater than k
    Called by the InputParams constructor
    """
    if k < 1:
        raise FuzzyError("k < 1")
    while True:
        k += 1
        if isprime(k):
            return k

def random_bytes(length: int) -> bytes:
    """
    Return a random byte string of length count.
    Called by the InputParams constructor
    """
    return secrets.token_bytes(length)

def check_words(words: List[int], set_size: int, corpus_size: int):
    """
    Check that the input words makes sense
    """
    if len(words) != set_size:
        raise FuzzyError("incorrect number of words")
    if len(set(words)) != set_size:
        raise FuzzyError("words are not unique")
    for word in words:
        if not 0 <= word < corpus_size:
            raise FuzzyError("word out of range")

def bytes_to_hex(data: bytes) -> str:
    """
    Return a hexadecimal string representation of an array of bytes
    where the string is made up of upper case hexadecimal
    characters. This function is the inverse of
    hex_to_bytes.

    bytes_to_hex(b'\xf8\x03') -> 'F803'
    """
    return binascii.hexlify(data).decode('utf-8').upper()

def bytes_to_hex_list(data: bytes, bpl: int) -> List[str]:
    """
    convert bytes into a list of hex strings
    where bpl (bytes per line) is the desired number
    of bytes represented in each string
    """
    j = len(data)
    k: int = (j + bpl - 1) // bpl
    return [bytes_to_hex(data[bpl * i: min(j, bpl * (i + 1))]) for i in range(k)]

def hex_list_to_bytes(hlist: List[str]) -> bytes:
    """
    converts a list of hex strings into bytes
    """
    return b''.join([hex_to_bytes(h) for h in hlist])

def hex_to_bytes(hex_repn: str) -> bytes:
    """
    Convert a hexadecimal string representation of
    a block of bytes into a Python bytes object.
    This function is the inverse of bytes_to_hex.

    hex_to_bytes('F803') -> b'\xf8\x03'
    """
    return binascii.unhexlify(hex_repn)

def test() -> None:
    """
    Tests gen_params, gen_secret and gen_keys
    """
    input_json = json.dumps(
        {
            "setSize": 12,
            "correctThreshold": 9,
            "corpusSize": 7776,
            "randomBytes": [
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                "0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF",
                ]
        }
    )
    params = gen_params(input_json)
    original = '[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ]'
    secret = gen_secret(params, original)
    print("original:", original)
    def test_guess(guess: str) -> None:
        "simple test"
        try:
            print("guess: {0}".format(guess))
            keys = gen_keys(secret, guess, 3)
            print("keys:")
            print(keys)
        except FuzzyError as e:
            print(e.message)

    test_guess('[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 110, 111, 112 ]')
    test_guess('[ 1, 2, 3, 4, 5, 6, 7, 8, 99, 110, 111, 112 ]')

if __name__ == '__main__':
    test()
