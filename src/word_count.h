//
// Created by Admin on 29.01.2021.
//

#ifndef UNTITLED4_WORD_COUNT_H
#define UNTITLED4_WORD_COUNT_H

#include <boost/algorithm/string.hpp>
#include <cctype>
#include <iostream>
#include <iomanip>
#include <map>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <boost/locale.hpp>
#include <boost/version.hpp>

std::unordered_map<std::string, size_t> text_to_vocabulary(std::string&& str, std::unordered_map<std::string, size_t> *res);
std::unordered_map<std::string, size_t> stringToMap(std::string const& text, std::unordered_map<std::string, size_t> *res);
void printMap(std::unordered_map<std::string, size_t> const& word_map);
void sortMap(std::unordered_map<std::string, size_t> const& word_map, int first_n=-1);
std::vector<std::pair<std::string, size_t>> sort_by_name(std::unordered_map<std::string, size_t>& d);
std::vector<std::pair<std::string, size_t>> sort_by_num(std::unordered_map<std::string, size_t>& d);
#endif //UNTITLED4_WORD_COUNT_H
