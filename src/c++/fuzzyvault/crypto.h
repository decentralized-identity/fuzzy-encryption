#ifndef _CRYPTO_H_
#define _CRYPTO_H_

#include "types.h"
#include "exceptions.h"
#include <vector>
#include <random>
#include <algorithm>
#include <iostream>

namespace crypto {

    /// The purpose of this object is for supplying random bytes supplied by the caller.
    /// It is also useful to produce repeatable results for debugging purposes.
    struct rng_t {
        bool _useBytes;
        size_t _i;
        std::vector<uint8_t> _randomBytes;

        /// copy of the random bytes supplied by the user to this object
        /// if the user specified the bytes
        rng_t(std::vector<uint8_t>* randomBytes)
        {
            _i = 0;
            _useBytes = false;
            if (randomBytes)
            {
                std::copy(
                    randomBytes->begin(),
                    randomBytes->end(),
                    std::back_inserter(_randomBytes));
                _useBytes = true;
            }
        }

        /// clear the copy and free the memory
        ~rng_t()
        {
            _randomBytes.clear();
        }

        /// generates a random byte by reading from the user supplied list
        uint8_t pop()
        {
            if (_i >= _randomBytes.size())
                throw Exception("rng_t: index out of range");
            if (!_useBytes)
                throw Exception("rng_t::pop used for normal case");
            uint8_t ans = _randomBytes[_i];
            _i += 1;
            return ans;
        }

        /// This indicates that the FuzzyVault code should use
        /// the random bytes supplied by the user rather than
        /// using a system call to generate random bytes.
        bool useBytes() const
        {
            return _useBytes;
        }
    };

    /// tests the primality of an integers
    ///
    /// @param n The integer to be tested. This number must be 
    ///         less than one million.
    /// @returns true if n is prime else false
    ///
    /// Reference: https://en.wikipedia.org/wiki/Primality_test
    bool is_prime(const int n);

    /// Returns the first prime strictly greater than
    /// a specfied integer
    ///
    /// @param k The exclusive lower bound on the prime
    int first_prime_greater_than(int k);

    /// returns a sha512 hash of an array of bytes
    ///
    /// @param data a reference to the array of bytes to be hashed
    /// @param out a reference to the array to receive the 64 byte
    ///         (512-bits) hash. The size of this array must be
    ///         greater than or equal to 64 bytes.
    void sha512(const std::vector<uint8_t>& data,
                std::vector<uint8_t>& out
                );

    /// returns a slow hash of a password
    /// @param pass an array of chars containing the password
    /// @param salt an array of bytes containing the salt
    /// @param out an array of bytes to receive the output hash
    void scrypt(const std::vector<uint8_t>& pass,
                const std::vector<uint8_t>& salt,
                std::vector<uint8_t>& out
                );

    /// returns the hash of an array of bytes using the given key
    ///
    /// @param key An array of bytes containing the key of the hash
    /// @param data An array of bytes containing the data to be hashed
    /// @param result An array of bytes where the resulting hash is stored.
    void hmac(const std::vector<uint8_t>& key,
              const std::vector<uint8_t>& data,
              std::vector<uint8_t>& result
              );

    /// Fills an array with random bytes
    ///
    /// If the user supplied random bytes then use them otherwise
    /// use a system call to generate the random bytes.
    ///
    /// @param rng pointer to a random bytes object
    /// @param bytes a reference to an array of bytes to be filled in
    /// by this function
    void random_bytes(rng_t* rng, std::vector<uint8_t>& bytes);

    /// Return a random integer greater than or equal to zero
    /// @param rng pointer to a random bytes object
    /// @returns a random positive integer
    int rand(rng_t* rng);

    /// get a random selection from a set 0 .. n-1 with no repeats
    ///
    /// @param rng pointer to a random bytes object
    /// @param n defines the range to be selected from as 0 .. n - 1
    /// @param m the number of numbers to be selected
    /// @returns a list of m randomly selected integers in the set 0 .. n - 1
    std::vector<int> rand_select(rng_t* rng, int n, int m);

}

#endif
