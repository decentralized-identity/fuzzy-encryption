#include <iostream>
#include <string>
#include "exceptions.h"
#include "fuzzy.h"

using namespace std;

void work()
{
    string input_string =
        "{\n"
        "  \"setSize\": 12,\n"
        "  \"correctThreshold\": 9,\n"
        "  \"corpusSize\": 7776\n"
        "}";
    
    string original_words = "[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 ]";
    // string recovery_words = "[ 1, 2, 3, 4, 5, 6, 7, 8, 9, 100, 110, 120 ]";
    string recovery_words = "[ 2, 1, 3, 4, 5, 6, 7, 8, 90, 10, 110, 120 ]";
    int key_count = 1;

    string params = fuzzy_vault::gen_params(input_string);
    string secret = fuzzy_vault::gen_secret(params, original_words);

    cout << endl << "input_string" << endl << input_string << endl;
    cout << endl << "original_words" << endl << original_words << endl;
    cout << endl << "recovery_words" << endl << recovery_words << endl;
    cout << endl << "params" << endl << params << endl;
    cout << endl << "secret" << endl << secret << endl;

    string keys = fuzzy_vault::gen_keys(secret, recovery_words, key_count);
    cout << endl << "keys" << endl << keys << endl;

    string recovery_words_2 = "[ 20, 1, 3, 4, 5, 6, 7, 8, 90, 10, 110, 120 ]";
    string keys2 = fuzzy_vault::gen_keys(secret, recovery_words_2, key_count);
    cout << endl << "keys2" << endl << keys2 << endl;

}

int main(int argc, char* argv[])
{
    try
    {
        work();
    }
    catch(const Exception& e)
    {
        std::cerr << e.what() << std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
    catch(const NoSolutionException& e)
    {
        std::cerr << "No Solution!" << std::endl;
    }
    
}
