#include <iostream>
#include <exception>
#include "fuzzy.h"

using namespace std;
using namespace fuzzy_vault;

/// 1. Create the parameters of the key recovery system
/// 2. Generate a secret with the original words
/// 3. Get the original keys using the original words
/// 4. Recover the keys using recovery words that are within
///    an allowable error tolerance of the original words
/// 5. Compare the original words with the recovered words
///    if they are different report failure
/// 6. Attempt to recover the keys using a recovery words
///    that have too many errors.
/// 7. If the recovery is successful report an error.
int work()
{
    string input_string =
        "{\n"
        "  \"setSize\": 9,\n"
        "  \"corpusSize\": 7776,\n"
        "  \"correctThreshold\": 6\n"
        "}";
    
    string original_words = "[  1, 2, 3, 4, 5, 6, 7, 8, 9 ]";
    // 3 errors -- within tolerance
    int key_count = 1;

    string params = gen_params(input_string);
    string secret = gen_secret(params, original_words);

    cout << endl << "input_string" << endl << input_string << endl;
    cout << endl << "params" << endl << params << endl;
    cout << endl << "secret" << endl << secret << endl;

    try
    {
        cout << endl << "original_words: " << original_words << endl;
        string original_keys = gen_keys(secret, original_words, key_count);
        cout << endl << "original keys: " << endl << original_keys << endl;

        string recovery_words = "[  1, 2, 3, 4, 5, 66, 77, 8, 99 ]";
        cout << endl << "recovery_words: " << recovery_words << endl;
        // 3 errors -- should be OK
        string recovered_keys = gen_keys(secret, recovery_words, key_count);
        cout << endl << "recovered keys: " << endl << recovered_keys << endl;
        if (recovered_keys != original_keys)
            return 1;

        string bad_words      = "[  1, 2, 3, 4, 5, 66, 77, 88, 99 ]";
        // 4 errors -- too much
        cout << endl << "bad_words: " << bad_words << endl;
        cout << "attempting to recover the keys ..." << endl;
        string bad_keys = gen_keys(secret, bad_words, key_count);
        return 2;  // should not get here
    }
    catch(const NoSolutionException)
    {
        cout << "keys could not be recovered -- as expected" << endl;
    }
    cout << endl << "All tests passed!" << endl;
    return 0;
}

int main(int argc, char* argv[])
{
    try
    {
        return work();
    }
    catch(const exception& e)
    {
        cerr << e.what() << '\n';
        return 3;
    }
}
