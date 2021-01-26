#include <sstream>
#include "fuzzy.h"
#include "params.h"
#include "secret.h"
#include "utils.h"
#include "input.h"
#include "crypto.h"
#include "imod.h"
#include "utils.h"
#include "exceptions.h"

std::string fuzzy_vault::gen_params(const std::string& input_string)
{
    input_t input(input_string);
    std::stringstream output;
    try
    {
        const int prime = crypto::first_prime_greater_than(input._corpusSize);
        std::vector<uint8_t>* randomBytes = input._randomBytes.size() > 0 ? &input._randomBytes : 0;
        params_t params(input._setSize, input._correctThreshold, input._corpusSize, prime, randomBytes);
        output << params;
    }
    catch(const std::exception& e)
    {
        imod_t::cleanup();
        throw;
    }
    imod_t::cleanup();
    return output.str();
}

std::string fuzzy_vault::gen_secret(const std::string& params_string,
                                    const std::string& words_string
                                    )
{
    std::stringstream output;
    params_t params(params_string);
    imod_t::initialize(params._prime);
    try
    {
        std::vector<int> words = utils::parse_ints(words_string);
        if (!utils::are_unique(words))
            throw Exception("gen_secret -- words are not unique");
        secret_t secret(params, words);
        output << secret;    
    }
    catch(const std::exception& e)
    {
        imod_t::cleanup();
        throw;
    }
    imod_t::cleanup();
        
    return output.str();
}

std::string fuzzy_vault::gen_keys(const std::string& secret_string,
                                  const std::string& recovery_words_string,
                                  int key_count
                                 )
{
    std::stringstream keys_stream;
    secret_t secret(secret_string);
    imod_t::initialize(secret._prime);
    try
    {
        std::vector<int> recovery_words = utils::parse_ints(recovery_words_string);
        if (recovery_words.size() != static_cast<size_t>(secret._setSize))
            throw Exception("gen_keys: incorrect number of recovery words");
        if (!utils::are_unique(recovery_words))
            throw Exception("gen_keys: recovery words are not unique");
        std::vector<std::vector<uint8_t>> keys;
        std::vector<int> recovered_words;

        secret.recover(recovery_words, recovered_words);
        secret.get_keys(recovered_words, key_count, keys);
        if (keys.size() == 0)
        {
            keys_stream << "[]";
        }
        else
        {
            keys_stream << "[" << std::endl;
            bool is_first = true;
            for(auto key : keys)
            {
                if (is_first)
                    is_first = false;
                else
                    keys_stream << "," << std::endl;
                keys_stream << "  \"" << key << "\"";
            }
            keys_stream << std::endl << "]";
        }
    }
    catch(const std::exception& e)
    {
        imod_t::cleanup();
        throw;
    }
    imod_t::cleanup();
    return keys_stream.str();
}