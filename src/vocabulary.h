#ifndef _SRC_VOCABULARY_H_
#define _SRC_VOCABULARY_H_

#include <cereal/archives/binary.hpp>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <vector>
#include "serialization.h"

using CountType = unsigned long long;
using WordFreq = std::pair<std::string, CountType>;
using WordMap = std::unordered_map<std::string, CountType>;
using Itoa = std::unordered_map<std::size_t, std::string>;
using Atoi = std::unordered_map<std::string, std::size_t>;

bool operator<(const WordFreq &w1, const WordFreq &w2);

class Vocabulary {
public:
    explicit Vocabulary(
        unsigned long mc = 1, CountType ms = 1e7, bool kc = false);
    Vocabulary(const Vocabulary &other);
    Vocabulary(Vocabulary &&other);

    void build(const std::vector<WordFreq> &v);
    void build(const std::string &file);

    void add(const std::string &word, CountType freq = 1);
    void remove(const std::string &word);
    void merge(const Vocabulary &other);
    void merge(const WordMap &other);
    void sort(const std::string &order = "desc");
    std::set<std::string> words() const;

    bool has(const std::string &word) const;
    bool has(const std::string &word, bool ignore_case) const;
    std::size_t size() const;
    bool full() const;
    void clear();

    void to_txt(const std::string &file) const;

    void serialize(cereal::BinaryOutputArchive &archive);
    void serialize(cereal::BinaryInputArchive &archive);

    WordMap::iterator begin();
    WordMap::iterator end();
    WordMap::const_iterator cbegin();
    WordMap::const_iterator cend();

    std::string operator[](const std::size_t &i) const;
    std::size_t operator[](const std::string &w) const;

    friend std::ostream &operator<<(std::ostream &os, const Vocabulary &vocab);

private:
    unsigned int min_count = 1;
    CountType max_size = 1e7;
    bool keep_case = false;
    WordMap freq;
    Itoa itoa;
    Atoi atoi;
};

#endif /* _SRC_VOCABULARY_H_ */
