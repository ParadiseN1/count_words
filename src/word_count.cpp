// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
//
// Created by Admin on 29.01.2021.
//

#include "word_count.h"

std::unordered_map<std::string, size_t> text_to_vocabulary(std::string&& str) {
    std::unordered_map<std::string, size_t> word_map;
    str = boost::locale::normalize(str);
    str = boost::locale::fold_case(str);
    boost::locale::boundary::ssegment_index map(boost::locale::boundary::word, str.begin(), str.end());
    map.rule(boost::locale::boundary::word_letters);
    for(auto split_itr = map.begin(), e = map.end(); split_itr != e; ++split_itr) {
        ++word_map[split_itr->str()];
    }
    return word_map;
}

std::unordered_map<std::string, size_t> stringToMap(std::string const& text, std::unordered_map<std::string, size_t> *res)
{
    std::cout << "stringToMap() starts... ";
    std::unordered_map<std::string, size_t> word_map;
    std::locale::global(boost::locale::generator().generate("en_US.UTF-8"));
    const char * kWhiteSpace = " ,.!?;\n\t\r()[]{}123567890-_=+`~\'\"\\/<>:@#$%^&*|";    //whatever you call white space
    boost::locale::normalize(text);
    auto new_txt = boost::algorithm::to_lower_copy(text); //copy
    auto token = std::strtok(new_txt.data(), kWhiteSpace);
    while(token) {
        word_map[token] += 1;
        token = std::strtok(nullptr, kWhiteSpace);
    }
    res = &word_map;
    std::cout << "complete.\n";
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

std::vector<std::pair<std::string, size_t>> sort_by_name(std::unordered_map<std::string, size_t>& d) {
    std::vector<std::pair<std::string, size_t>> vec;

    // copy key-value pairs from the map to the vector
    auto it2 = d.begin();
    for (; it2!=d.end(); it2++)
    {
        vec.emplace_back(make_pair(it2->first, it2->second));
    }
    auto comparator = [](const std::pair<std::string, size_t>& p1, const std::pair<std::string, size_t>& p2) { return p1.first < p2.first; };
    std::sort(vec.begin(), vec.end(), comparator);
    return vec;
}

std::vector<std::pair<std::string, size_t>> sort_by_num(std::unordered_map<std::string, size_t>& d) {
    std::vector<std::pair<std::string, size_t>> vec;

    // copy key-value pairs from the map to the vector
    auto it2 = d.begin();
    for (; it2!=d.end(); it2++)
    {
        vec.emplace_back(make_pair(it2->first, it2->second));
    }
    auto comparator = [](const std::pair<std::string, size_t>& p1, const std::pair<std::string, size_t>& p2)  { return p1.second > p2.second; };
    std::sort(vec.begin(), vec.end(), comparator);
    return vec;
}

void sortMap(std::unordered_map<std::string, size_t> & word_map, int first_n){
    std::vector<std::pair<std::string, size_t>> vec;

    // copy key-value pairs from the map to the vector
    auto it2 = word_map.begin();
    for (; it2!=word_map.end(); ++it2)
    {
        vec.emplace_back(make_pair(it2->first, it2->second));
    }
    sort(vec.begin(), vec.end(), sortByVal);
//    if (print)
    printVecMap(vec, first_n);
}