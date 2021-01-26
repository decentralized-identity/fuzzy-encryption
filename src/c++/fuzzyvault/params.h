#ifndef _PARAMS_H_
#define _PARAMS_H_
#include <ostream>
#include <stdint.h>
#include <vector>


/// This structure contains the parameters of the key recovery
/// scheme that is common to all secrets.
struct params_t {
    int _setSize;            ///< number of words (integers) needed to 
                            ///< generate keys

    int _correctThreshold;   ///< The minimum number of matches between
                            ///< the original and recovery words needed
                            ///< to recover the keys

    int _corpusSize;         ///< the number of words in the allowable set

    int _prime;              ///< the prime number to be used in all calculations

    std::vector<int> _extractor; ///< A set of number used in key generation
    std::vector<uint8_t> _salt;  ///< a set of bits used for the has algorithm

    /// constructor for the paramters
    /// @param setSize number of words in a secret
    /// @param correctThreshold number of required matches to generate keys
    /// @param corpusSize number of available words to choose from
    /// @
    params_t(int setSize,
             int correctThreshold,
             int corpusSize,
             int prime,
             std::vector<uint8_t>* randomBytes = 0
            );
    params_t(std::string json_string);
    ~params_t();
    void clear();
};

std::ostream& operator<<(std::ostream& os, const params_t& params);

#endif