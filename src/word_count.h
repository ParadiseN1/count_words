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

std::unordered_map<std::string, u_int> stringToMap(std::string const& text, std::unordered_map<std::string, u_int> *res);
void printMap(std::unordered_map<std::string, u_int> const& word_map);
void sortMap(std::unordered_map<std::string, u_int> const& word_map, int first_n=-1);
#endif //UNTITLED4_WORD_COUNT_H
