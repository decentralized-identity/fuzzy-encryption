#include "stdlib.h"
#include "limits.h"
#include "utils.h"
#include <iomanip>
#include <fstream>
#include <algorithm>
#include "types.h"
#include "json.h"
#include "exceptions.h"

#if 0
void utils::write_text(const std::string& path,
                       const std::stringstream& s
                      )
{
    std::ofstream file(path);
    file << s.rdbuf();
}

std::stringstream utils::read_text(const std::string& path)
{
    std::stringstream ans;
    std::ifstream file(path);
    ans << file.rdbuf();
    return ans;
}
#endif

std::vector<int> utils::parse_ints(const std::string& json)
{
    std::vector<int> output;
    struct json_value_s* root = json_parse(json.c_str(), json.length());
    if (root == 0)
        throw Exception("utils::parse_ints -- json_parse failed");
    try
    {
        if (root->type != json_type_array)
            throw Exception("utils::parse_ints -- not an array");
        json_array_s* array = (json_array_s*)root->payload;
        if (array == 0)
            throw Exception("utils::parse_ints -- array == 0");
        for (json_array_element_s* E = array->start; E; E = E->next)
        {
            json_value_s* V = E->value;
            if (V == 0)
                throw Exception("utils::parse_ints: V == 0");
            if (V->type != json_type_number)
                throw Exception("utils::parse_ints -- value is not a number");
            if (V->payload == 0)
                throw Exception("utils::parse_ints -- payload is zero");
            json_number_s* N = (json_number_s*)(V->payload);
            if (N->number == 0)
                throw Exception("utils::parse_ints -- N->number is null");
            char* endptr = 0;
            long int n = strtol(N->number, &endptr, 10);
            if (n == LONG_MAX || n == LONG_MIN || (n == 0 && errno != 0))
                throw Exception("utils::parse_ints -- bad integer");
            output.push_back(n);
        }
    }
    catch (...)
    {
        free(root);
        throw;
    }
    free(root);

    return output;
}

bool utils::are_unique(const std::vector<int>& xs)
{
    if (xs.size() == 0 || xs.size() == 1)
        return true;
    std::vector<int> Xs = xs;
    std::sort(Xs.begin(), Xs.end());
    int x = Xs[0];
    for (auto it = Xs.begin() + 1; it != Xs.end(); it++)
    {
        if (*it == x)
            return false;
        x = *it;
    }
    return true;
}
