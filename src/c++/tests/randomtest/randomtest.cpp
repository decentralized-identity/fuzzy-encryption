/// Tests the Fuzzy Vault Key Recovery scheme by randomly generating
/// original sets of integers (words), then randomly mutating those
/// integers to create a mutant list of integers that are used to 
/// attempt a key recovery. If the number of mutations is small 
/// we check to see if the key is recovered and if the number of 
/// mutations is large then we check to see that the key is not
/// recovered. The number of mutations is chosen randomly but it
/// favors a small number of mutations over a large thus simulating
/// guesses with some knowledge of the original words.
///
/// Usage:
///
///     ./randomtest <set-size> <correct-threshold> <coprus-size> <test-count>
///
///     <set-size> is the number of integers (words) comprising the secret
///     <correct-threshold> is the minimum number of matches of the
///         original integers int the secret that the guess must match
///         in order to recover the keys
///     <corpus-size> The size of the available universe of integers (words)
///         All numbers in the original and guesses must be in the range
///         0 ... <corpus-size>-1
///     <test-count> The number of random tests to be performed.
///             

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <set>
#include <random>
#include <fstream>
#include <unordered_set>
#include <iomanip>
#include <string>
#include "fuzzy.h"
#include "exceptions.h"

using namespace std;

typedef uniform_int_distribution<int> randint_t;
typedef default_random_engine randeng_t;

/// For writing int vectors to a stream in the form
/// of a JSON list
ostream& operator<<(ostream& os, const vector<int>& xs)
{
    bool is_first = true;
    os << "[";
    for (int x: xs)
    {
        if (!is_first)
            os << ",";
        else
            is_first = false;
        os << " " << x;
    }
    os << " ]";
    return os;
}

/// Convert a list of ints into a string containing
/// a JSON representation of the list
string vector_int_to_json(const vector<int>& xs)
{
    stringstream ss;
    ss << xs;
    return ss.str();
}

/// Converts a set of integers to a vector of integers with
/// the same members
vector<int> set_to_vector(const set<int>& xs)
{
    vector<int> ans;
    ans.assign(xs.begin(), xs.end());
    return ans;
}

/// Returns the set of integers contained in a vector of integers
set<int> vector_to_set(const vector<int>& xs)
{
    set<int> ans;
    for (int x : xs)
        ans.insert(x);
    return ans;
}

/// Returns the intersection of two sets of integers
set<int> set_intersect(const set<int>& xs, const set<int>& ys)
{
    set<int> ans;
    for (int x : xs)
    {
        if (ys.count(x) > 0)
            ans.insert(x);
    }
    return ans;
}

/// Determines if all of the elements in an integers vector are unique
bool vector_contains_unique_values(const vector<int>& xs)
{
    set<int> ys = vector_to_set(xs);
    return ys.size() == xs.size();
}

/// Returns the number of elements that match between two integer vectors.
/// It is assumed that each vector has no repeats withing itself.
size_t vector_intersection_count(const vector<int>& xs, const vector<int>& ys)
{
    set<int> x_set = vector_to_set(xs);
    set<int> y_set = vector_to_set(ys);
    return set_intersect(x_set, y_set).size();
}

/// Return a vector consisting of a decending sequence
/// of integers of a specified size. The sequence ends in 1.
/// This function is called to help generated a distribution
/// of random mutation counts.
///
///      {size-1, size-2, ..., 1}
///
/// @param size length of the sequence
vector<int> get_lengths(int size)
{
    vector<int> lengths = {0};
    for (int i = 0; i <  size - 1; i++)
        lengths.push_back(size - i - 1);
    return lengths;
}


/// This function returns a vector consisting of the running
/// total of integers in the input vector. It is the analog
/// of an integral. It is used in generating random mutation
/// counts
vector<int> get_sums(const vector<int>& lengths) 
{
    vector<int> ans;
    int sum = 0;
    for (int x: lengths) 
    {
        sum += x;
        ans.push_back(sum);
    }
    return ans;
}

/// Given an sum return which bin if falls into. The bin
/// number serves as a mutation count. By randomly 
/// generating an integer between 1 and the maximum
/// value in the monotonically increasing sums list, we
/// get a random bin index. The larger bins have a better
/// chance of being selected. Thus we can generated
/// a random integer with an arbitrary distribution.
int get_bin(const vector<int>& sums, int sum)
{
    const int count = static_cast<int>(sums.size());
    int left = 0;
    for (int i = 0; i < count; i++)
    {
        int right = sums[i];
        if (left < sum && sum <= right)
            return i;
        left = right;
    }
    throw "bin failed";
}

/// This returns a random mutation cout using the distribution
/// as defined in the sums vector. We generate a random number
/// withing the allowed distribution and then find in which
/// bin it lies. This is the random mutation count.
int get_random_mutation_count(
    mt19937& gen,
    randint_t& random_length,
    const vector<int>& sums
)
{
    const int length = random_length(gen);
    return get_bin(sums, length);
}

// string read_file(const string& path)
// {
//     ifstream t(path);
//     stringstream ans;
//     ans << t.rdbuf();
//     return ans.str();
// }

/// Returns a vector of random integers in the range 0 .. corpus_size - 1
/// No integer is repeated
///
/// @param set_size The number of integers to be returned
/// @param corpus_size Specifies the range of integers to be returned
/// @param gen random number generator
vector<int> get_random_words(
    const int set_size,
    const int corpus_size,
    mt19937& gen
)
{
    vector<int> deck;
    for (int i = 0; i < corpus_size; i++)
        deck.push_back(i);
    shuffle(deck.begin(), deck.end(), gen);
    vector<int> ans;
    for (int i = 0; i < set_size; i++)
        ans.push_back(deck[i]);
    return ans;
}

/// There are three possible outcomes to a key recovery.
/// The keys can be successfully recovered, the keys
/// are not recovered because the number of matches was
/// not sufficient and finally, something went terribly
/// wrong for example there were repeated words in the guess
typedef enum _prediction {
    prediction_success,
    prediction_failure,
    prediction_no_solution
} prediction_t;

/// Given the number of mutations and the correction threshold
/// we can predict the outcome of a key recovery. This 
/// function returns that prediction. We expect the keys
/// to be recovered if the number of correct words in the
/// mutation is greater than or equal to the correct threshold.
/// If any of the mutated words are repeated then we expect
/// a catstrophic failure.
prediction_t get_prediction(
    const int set_size,
    const int correct_threshold,
    const int mutation_count,
    bool repeats
)
{
    if (repeats)
        return prediction_failure;
    if (correct_threshold + mutation_count <= set_size)
        return prediction_success;
    return prediction_no_solution;
}

/// Return the JSON string corresponding to the secret given
/// the input parameters. In normal practice, this secret is 
/// stored on behalf of the user to be used later in
/// key recovery.
string get_secret(
    const int set_size,
    const int correct_threshold,
    const int corpus_size,
    const vector<int>& originals
)
{
    stringstream input_stream;
    input_stream
        << "{" << endl
        << "  \"setSize\" : " << set_size << "," << endl
        << "  \"correctThreshold\" : " << correct_threshold << "," << endl
        << "  \"corpusSize\" : " << corpus_size << endl
        << "}";

    string input = input_stream.str();
    string params = fuzzy_vault::gen_params(input);
    return fuzzy_vault::gen_secret(params, vector_int_to_json(originals));
}

/// Returns true if any element in the list of integers is repeated
bool repeats_found(const vector<int>& mutated)
{
    return !vector_contains_unique_values(mutated);
}

/// On input the mutated vector is normally shorter than the original
/// vector. The mutated vector is filled out to match the size
/// of the original by taking members of the original. In this way
/// the mutated vector is a mutation of the original.
void fill_out_mutated(vector<int>& mutated, const vector<int>& originals)
{
    const size_t set_size = originals.size();
    const size_t mutation_count = mutated.size();
    for (size_t i = 0; i < set_size - mutation_count; i++)
        mutated.push_back(originals[i]);
}

/// This function is called when the prediction does not match
/// experiment. The data printed can be used to diagnose the 
/// problem at a later date.
void print_example(
    const string& secret,
    const vector<int>& originals,
    const vector<int>& mutants
)
{
    cout << endl;
    cout << "secret <- " << secret << endl;
    cout << "original_words : " << vector_int_to_json(originals) << endl;
    cout << "mutated_words  : " << vector_int_to_json(mutants) << endl;
    cout << endl;
    cout.flush();
}

/// Starting with the original words and the information provided, generate
/// a secrect. Then create a set of recovery words by randomly mutating the
/// original words. Attempt to recover the keys using the mutated words
/// and then match the result of that recovery against a prediction based
/// upon the corrrect threshold of the scheme. If the prediction fails,
/// print out the information for later use.
int mutate_test(
    const vector<int>& originals,
    const int correct_threshold,
    const int corpus_size,
    mt19937& gen,
    randint_t& random_length,
    randint_t& random_word,
    const vector<int>& sums
)
{
    const int set_size = static_cast<int>(originals.size());
    const int n_mutations = get_random_mutation_count(gen, random_length, sums);
    const string secret = get_secret(set_size, correct_threshold, corpus_size, originals);
    vector<int> mutants = get_random_words(n_mutations, corpus_size, gen);
    fill_out_mutated(mutants, originals);
    const int intersection_count = vector_intersection_count(mutants, originals);
    const int error_count = set_size - intersection_count;
    const bool repeats_exist = repeats_found(mutants);

    const string words = vector_int_to_json(mutants);

    switch (get_prediction(set_size, correct_threshold, error_count, repeats_exist))
    {
        case prediction_success:
            try
            {
                string keys = fuzzy_vault::gen_keys(secret, words, 1);
            }
            catch(fuzzy_vault::NoSolutionException)
            {
                cout << "prediction_success -- NoSolution" << endl;
                print_example(secret, originals, mutants);
                return 1;
            }
            catch(...)
            {
                cout << "prediction_success -- failure" << endl;
                print_example(secret, originals, mutants);
                return 2;
            }
            
            break;
        case prediction_no_solution:
            try
            {
                string keys = fuzzy_vault::gen_keys(secret, words, 1);
                cout << "prediction_no_solution -- success" << endl;
                print_example(secret, originals, mutants);
            }
            catch(fuzzy_vault::NoSolutionException)
            {
                ;
            }
            catch(const exception& e)
            {
                cout << "prediction_no_solution -- failure" << endl;
                print_example(secret, originals, mutants);
                return 3;
            }
            break;
        case prediction_failure:
            try
            {
                string keys = fuzzy_vault::gen_keys(secret, words, 1);
                cout << "prediction_failure -- success" << endl;
                print_example(secret, originals, mutants);
            }
            catch(fuzzy_vault::NoSolutionException)
            {
                cout << "prediction_failure -- NoSolution" << endl;
                print_example(secret, originals, mutants);
                return 4;
            }
            catch(...)
            {
                return 5;
            }
            break;
    }
    return 0;
}

/// Test the key recovery scheme using the original words as the
/// recovery words. This better work!
int exact_match_test(
    const int set_size,
    const int correct_threshold,
    const int corpus_size,
    const vector<int>& originals
)
{
    string secret = get_secret(set_size, correct_threshold, corpus_size, originals);
    try
    {
        string keys = fuzzy_vault::gen_keys(secret, vector_int_to_json(originals), 1);
        return 0;
    }
    catch(...)
    {
        print_example(secret, originals, originals);
        return 6;
    }
}

/// Perform a single test which consists of first using the origina words
/// as a the recovery words and then mutating the original words and 
/// then using the mutation to recovery the keys.
int test(
    const int set_size,
    const int correct_threshold,
    const int corpus_size,
    mt19937& gen,
    randint_t& random_length,
    randint_t& random_word,
    const vector<int>& sums
    )
{
    vector<int> originals = get_random_words(set_size, corpus_size, gen);
    exact_match_test(set_size, correct_threshold, corpus_size, originals);
    if (mutate_test(originals, correct_threshold, corpus_size, gen, random_length, random_word, sums) != 0)
        return 7;
    return 0;
}

/// Return a random number generator to be used in generating random mutations.
/// This takes a list of lengths which represents a set of relative 
/// probablities of a the corresponding index of the length. Smaller lengths
/// have smaller probabilities. To do this the running sum of the lengths
/// is placed in the list called sums. The last element of sums represents
/// the sum total of all all the lengths. Return an random number generator
/// that randomly returns a number between 1 and the total length.
randint_t get_random_length(const int set_size, vector<int>& sums)
{
    vector<int> lengths = get_lengths(set_size);
    sums = get_sums(lengths);
    const int last = sums[sums.size() - 1];
    randint_t ans(1, last);
    return ans;
}

/// Return an random number generator for the integers in the corpus.
randint_t get_random_word(const int corpus_size)
{
    randint_t ans(0, corpus_size - 1);
    return ans;
}

/// Return the underlying random number generator
mt19937 get_gen()
{
    random_device rd;
    mt19937 gen(rd());
    return gen;
}


/// Calls the requested number of tests
#define ROW_LENGTH 100
int work(const int set_size, const int correct_threshold, const int corpus_size, const int test_count)
{
    mt19937 gen = get_gen();
    vector<int> sums;
    randint_t random_length = get_random_length(set_size, sums);
    randint_t random_word = get_random_word(corpus_size);

    int rc = 0;
    int j = 0;
    for (int i = 0; i < test_count; i++)
    {
        j++;
        cout << ".";
        cout.flush();
        if (j == ROW_LENGTH)
        {
            cout << setw(12) << (i + 1) << endl;
            cout.flush();
            j = 0;
        }
        rc = test(set_size, correct_threshold, corpus_size, gen, random_length, random_word, sums);
    }
    cout << endl;
    return rc;
}
#undef ROW_LENGTH

/// A structure containing the arguments to this program
struct args_t {
    int _set_size;
    int _correct_threshold;
    int _corpus_size;
    int _test_count;

    args_t(int argc, char* argv[]);
};

/// Converts a string to an integer. If the string does not
/// have the format of a decimal number it throws an exception.
int dec_to_int(const char* s)
{
    char* endptr = 0;
    long ans = strtol(s, &endptr, 10);
    if (*endptr != 0)
        throw Exception("invalid integer");
    return static_cast<int>(ans);
}

/// Constructor for the arguments to this program.
args_t::args_t(int argc, char* argv[]) : _set_size(0), _corpus_size(0)
{
    if (argc != 5)
        throw Exception("incorrect number of arguments");
    _set_size = dec_to_int(argv[1]);
    _correct_threshold = dec_to_int(argv[2]);
    _corpus_size = dec_to_int(argv[3]);
    _test_count = dec_to_int(argv[4]);
}

/// For writing this arguments to a stream
ostream& operator<<(ostream& os, const args_t& args)
{
    os
        << "{" << endl
        << "  \"_set_size\" : " << args._set_size << "," << endl
        << "  \"_correct_threshold\" : " << args._correct_threshold << "," << endl
        << "  \"_corpus_size\" : " << args._corpus_size << "," << endl
        << "  \"_test_count\" : " << args._test_count << endl
        << "}";

    return os;
}

/// Main entry point.
int main(int argc, char* argv[])
{
    try
    {
        args_t args(argc, argv);
        cout << args << endl;
        return work(args._set_size,
                    args._correct_threshold,
                    args._corpus_size,
                    args._test_count);
    }
    catch(const std::exception& e)
    {
        cerr 
            << e.what() << endl
            << endl
            << "Usage" << endl
            << endl
            << "   ./randomtest <set-size> <correct-threshold> <corpus-size> <test-count>" << endl
            << endl;
        return 8;
    }
    
    return 0;
}
