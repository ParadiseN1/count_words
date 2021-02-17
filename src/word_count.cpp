//
// Created by Admin on 29.01.2021.
//

#include "word_count.h"

std::unordered_map<std::string, size_t> stringToMap(std::string const& text, std::unordered_map<std::string, size_t> *res)
{
    std::unordered_map<std::string, size_t> word_map;
    std::locale::global(boost::locale::generator().generate("en_US.UTF-8"));
    const char* kWhiteSpace = " ,.!?;\n\t\r()[]{}123567890-_=+`~\'\"\\/<>:@#$%^&*|";    //whatever you call white space
    boost::locale::normalize(text);
    auto new_txt = boost::algorithm::to_lower_copy(text); //copy
    char* token = std::strtok((char *)new_txt.data(), kWhiteSpace);
    while(token) {
        word_map[token] += 1;
        token = std::strtok(nullptr, kWhiteSpace);
    }
    res = &word_map;
    return word_map;
}

void printMap(std::unordered_map<std::string, size_t> const& word_map)
{
    for (auto const& [key, val] : word_map)
    {
        std::cout << key        // string (key)
                  << ':'
                  << val        // string's value
                  << std::endl;
    }
}

void printVecMap(std::vector<std::pair<std::string, size_t>> const& word_map, int first_n = -1)
{
    for (auto const& [key, val] : word_map)
    {
        if (first_n != -1)
            first_n--;
        std::cout << key        // string (key)
                  << ':'
                  << val        // string's value
                  << std::endl;
        if (first_n == 0)
            return;
    }
}

bool sortByVal(const std::pair<std::string, size_t> &a,
               const std::pair<std::string, size_t> &b)
{
    if (a.second == b.second)
        return (a.first < b.first);
    return (a.second > b.second);
}

void sortMap(std::unordered_map<std::string, size_t> const& word_map, int first_n){
    std::vector<std::pair<std::string, size_t>> vec;

    // copy key-value pairs from the map to the vector
    auto it2 = word_map.begin();
    for (; it2!=word_map.end(); it2++)
    {
        vec.emplace_back(make_pair(it2->first, it2->second));
    }
    sort(vec.begin(), vec.end(), sortByVal);
//    if (print)
    printVecMap(vec, first_n);
}