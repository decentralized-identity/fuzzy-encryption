#ifndef _INPUT_H_
#define _INPUT_H_

#include <string>
#include <iostream>
#include <vector>
#include <stdint.h>

/// This structure defines the parameters of the key recovery scheme
/// This information will be passed to gen_secret()
struct input_t {
    int _setSize;            ///< The number of words required to generate keys
    int _correctThreshold;   ///< The number of correct words required to generate keys
    int _corpusSize;         ///< The number of word to choose from
    std::vector<uint8_t> _randomBytes;

    /// Initial constructor
    /// @param setSize value for setSize
    /// @param correctThreshold value for correctThreshold
    /// @param corpusSize value for corpusSize
    input_t(int setSize,
            int correctThreshold,
            int corpusSize
            ) : _setSize(setSize), _correctThreshold(correctThreshold), _corpusSize(corpusSize)
            {}

    /// Construct from a JSON representation
    /// @param json_string a string containing the representation
    input_t(const std::string& json_string);

    ~input_t()
    {
        _randomBytes.clear();
    }
};

std::ostream& operator<<(std::ostream& os, const input_t& input);

#endif
