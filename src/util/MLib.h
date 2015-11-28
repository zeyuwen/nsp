#ifndef JIAN_MLIB_H
#define JIAN_MLIB_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <iterator>

using namespace std;

namespace jian {

template <typename T, size_t N>
char (&_ArraySizeHelper(T (&array)[N]))[N];

template <typename Array>
std::size_t count(Array array) {
    return sizeof _ArraySizeHelper(array);
}

void tokenize(const string &, vector<string> &, const string & = " ");
void tokenize(const string &, vector<string> &, const string &, const string &temp);
string upper(string);
string lower(string);
int die(string str);
std::string env(std::string);

} /// namespace jian

#endif





