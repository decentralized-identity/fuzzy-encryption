// #ifdef __cplusplus
//  extern "C" {
// #endif

#ifndef _FUZZY_H_
#define _FUZZY_H_


#define FUZZYLIB_API_EXPORT __attribute__((visibility("default")))

#include <string>



/**
* Fuzzy Vault exports
* 
* All arguments and return values are in the form of std::stringstream objects.
* The strings contain JSON representation of data. It is assumed that the
* caller has JSON parsing support.
*/
namespace fuzzy_vault {
    /// This exception is thrown when the keys cannot
    /// be recovered because the recovery words are 
    /// not sufficient. This is a normal event and
    /// should not cause the process to abort.
    /// Instead the user should be informed that the
    /// recovery words are insufficient.
    class NoSolutionException {};


    /** Generates parameters to be passed to gen_secret()

    @param params_input
    Contains a JSON string representing a dictionary of the
    following form. I refer to this information as the \b input .

        {
            "setSize" : 12,
            "correctTreshold" : 9,
            "corpusSize" : 7776
        }

    @returns A string containing a JSON dictionary of the following form

        {
          "setSize": 12,
          "correctThreshold": 9,
          "corpusSize": 7776,
          "prime": 7789,
          "extractor": [ 1223, 81, 1257, 2529, 2115,  ... 5130, 416 ],
          "salt": "CF339C756CFAA7715018C8FFF97343454  ... 94DABBC8D36"
        }

    This string is opaque to the caller which I shall refer to as the \b parameters.
    The parameters are to be passed to gen_secret() .

    This function will be included in our delivery.
    */
    FUZZYLIB_API_EXPORT  std::string gen_params(const std::string& params_input);

    /** Generates a secret to be passed into gen_keys()

    The secret must be stored unmodified to be used as an argument to
    gen_keys() at a later time.

    @param params A JSON string returned by gen_params()

    @param words A JSON string representing a list of *setSize* unique integers
    in the range 0 .. *corpusSize* - 1 as specified in *params*.
    The words JSON string looks like

        { 78, 2643, 1178, ... }

    @returns A string containing the \b secret which has the following form

         {
           "setSize": 12,
           "correctThreshold": 9,
           "corpusSize": 7776,
           "prime": 7789,
           "extractor": [ 1223, 81, 1257, 2529, ...  5130, 416 ],
           "salt": "CF339C756CFAA7715018C8FFF97343 ... DABBC8D36",
           "sketch": [ 967, 5576, 1719, 6542, 2717, 7711 ],
           "hash": "73E8AB1883CB093F1C546D69DC87EC0FE658 ... FA975745"
        }

    It is up to the application to store the secret and guaranteed that
    it will not be modified. The secret will be one of the arguments
    passed to gen_keys(). This information alone is not enough to
    recover the words passed into this function so the application
    need not encrypt this information, however, consult your
    local cryptographer for advice.

    This function will be included in our delivery.
    */
    FUZZYLIB_API_EXPORT std::string gen_secret(const std::string& params,
                                 const std::string& words
                                );

    /** Generates a list of keys

    To generate keys the caller must supply a set words (integers) that closely
    matches the words specified in gen_secret(). This means that number
    of words must be unique, the number of words must be equal to setSize,
    every word must be greater than or equal to zero and less than corpusSize and
    the number of words matching the original set must be greater that
    or equal to correctThreshold. If all of these conditions are met
    then a list of keys of size key_count is returned to the user
    in the form of a JSON string.

    All calls to this function will return the same sequence of keys.
    After all, it is a function.

    @param secret A JSON string returned by gen_secret().

    @param words a list of unique integers. These integers represent a guess
    of the original words passed into gen_secret().

    @param key_count a positive integer specifying the number of keys to be returned

    @returns A JSON string representing a list of recovered keys. The returned
    string has the form

        [
            "B4263013BC29B964F6FB62FEB7119 ... ACBDC55A8C24A4ED78185936E76C8CD",
            "23568650436339CCA498D396D9EFD ... BB4B1CD2D97D869A3745080E323D62F",
            "E4CCD887D50179DD4B0BB57E95010 ... 4A35C1AA2B4C606B82C319C8A1D9B61",
            "6FEABCC8DDD8FA6557C7D096FA612 ... 1419E5EE7F0ED739CA9FA4E03393E44",
            "B075330F188F8C1795B715165B67F ... D11FC1B2D206D2E29D99EE3A020B150"
        ]

    Each key is represented as a large hexadecimal string all upper case.
    Each string is a representation of an array of bytes. Each bytes
    is represented by two consecutive hexadecimal characters, the lowest
    byte starting at the left. A byte is as represented by two characters in
    the 'obvious' way. For example '08' represents a byte value of 8.
    Typically the keys represent 512 bits or 64 bytes so they each
    have a length of 128 characters.

    This function will be included in our delivery.

    */
    FUZZYLIB_API_EXPORT std::string gen_keys(const std::string& secret,
                               const std::string& words,
                               int key_count
                              );
};

#endif

// #ifdef __cplusplus
// }
// #endif