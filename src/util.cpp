#include "util.h"
#include <algorithm>
#include <iostream>
#include <ostream>
#include <sstream>
#include <vector>

std::vector<std::string> split(const std::string &str, const char &delimiter) {
    std::istringstream iss(str);
    std::string word;
    std::vector<std::string> words;

    while (getline(iss, word, delimiter)) {
        if (!word.empty()) {
            words.push_back(word);
        }
    }

    return words;
}

std::string join(const std::vector<std::string> &strs, const char &delimiter) {
    if (strs.empty()) {
        return "";
    }

    std::string res = strs[0];
    for (auto iter = strs.begin() + 1; iter != strs.end(); ++iter) {
        if (delimiter) {
            res += delimiter;
        }
        res += *iter;
    }
    return res;
}

std::string join(
    std::initializer_list<std::string> lst, const char &delimiter) {
    return join(std::vector<std::string>(lst), delimiter);
}

std::string lower(const std::string &str) {
    std::string res(str);
    std::transform(str.begin(), str.end(), res.begin(), ::tolower);

    return res;
}

std::string path::join(
    const std::vector<std::string> &strs, const char &delimiter) {
    std::vector<std::string> normlized(strs.size());

    std::transform(
        strs.begin(), strs.end(), normlized.begin(),
        [&delimiter](const std::string &s) {
            std::size_t p = s.find_first_not_of(delimiter);
            if (p == std::string::npos) {
                return std::string("");
            }
            std::size_t q = s.find_last_not_of(delimiter);
            return q == std::string::npos ? s.substr(p)
                                          : s.substr(p, q + 1 - p);
        });

    return ::join(normlized, '/');
}

std::string path::join(
    std::initializer_list<std::string> lst, const char &delimiter) {
    return join(std::vector<std::string>(lst), delimiter);
}
