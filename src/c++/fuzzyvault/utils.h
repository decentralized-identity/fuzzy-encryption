#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <vector>


/** 
* Utilities used for the demonstration code 
*/
namespace utils {
/**
* Write a string to a file
*
* @param path The path to the destination file
* @param s The string to be written to the file
* @returns nothing
* 
* This function has been included for demonstration purposes.
* This function will not be included in our delivery.
*/
void write_text(const std::string& path,
                const std::stringstream& s
               );

/**
* Read a string from a file
*
* @param path path to the text file to be read
* @returns a string containing a copy of the text in the file
*
* This function has been included for demonstration purposes.
* This function will not be included in our deliver.
*/
std::stringstream read_text(const std::string& path);

/**
 * Parse a JSON string assuming it is a list of integers
 * 
 * @param input a JSON string
 * @return a vector of ints
 */
std::vector<int> parse_ints(const std::string& input);

/**
 * checks that the all of the integers are unique
 * 
 * @param xs The numbers to be checked
 * @returns true if there are not repeats
 */
bool are_unique(const std::vector<int>& xs);
};

#endif
