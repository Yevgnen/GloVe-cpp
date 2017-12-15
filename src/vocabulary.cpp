#include "vocabulary.h"
#include <algorithm>
#include <cereal/types/map.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/unordered_map.hpp>
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <unordered_map>
#include "util.h"

bool operator<(const WordFreq &w1, const WordFreq &w2) {
    return w1.second < w2.second || w1.first < w2.first;
}

// Vocabulary
Vocabulary::Vocabulary(unsigned long mc, CountType ms, bool kc)
    : min_count(mc), max_size(ms), keep_case(kc) {}

Vocabulary::Vocabulary(const Vocabulary &other)
    : min_count(other.min_count),
      max_size(other.max_size),
      keep_case(other.keep_case),
      freq(other.freq),
      itoa(other.itoa),
      atoi(other.atoi) {}

Vocabulary::Vocabulary(Vocabulary &&other)
    : min_count(other.min_count),
      max_size(other.max_size),
      keep_case(other.keep_case),
      freq(std::move(other.freq)),
      itoa(std::move(other.itoa)),
      atoi(std::move(other.atoi)) {}

void Vocabulary::build(const std::vector<WordFreq> &v) {
    clear();
    for (const auto &p : v) {
        add(p.first, p.second);
    }
}

void Vocabulary::build(const std::string &file) {
    std::ifstream ifs;
    file::open(ifs, file);

    std::string line;
    std::unordered_map<std::string, unsigned int> counts;
    std::string key;

    // Statistics
    while (getline(ifs, line)) {
        std::vector<std::string> &&_words = split(line);

        for (const auto &word : _words) {
            key = keep_case ? word : lower(word);
            ++counts[key];
        }
    }
    ifs.close();

    // Remove low frequencies
    std::vector<WordFreq> vec;
    std::copy_if(
        counts.begin(), counts.end(), std::back_inserter(vec),
        [&](const WordFreq &w) { return w.second >= this->min_count; });

    // Trim to max size
    std::sort(
        vec.begin(), vec.end(), [](const WordFreq &w1, const WordFreq &w2) {
            return w1.second > w2.second;
        });
    vec.erase(min(vec.begin() + max_size, vec.end()), vec.end());

    return build(vec);
}

void Vocabulary::add(const std::string &word, CountType freq) {
    std::string key = keep_case ? word : lower(word);
    if (has(key, true)) {
        this->freq[key] += freq;
    } else if (!full()) {
        std::size_t id = size();
        this->freq[key] += freq;
        this->atoi[key] = id;
        this->itoa[id] = key;
    }
    return;
}

bool Vocabulary::has(const std::string &word) const {
    return freq.count(keep_case ? word : lower(word)) > 0;
}

bool Vocabulary::has(const std::string &word, bool ignore_case) const {
    // If `ignore_case` is `true`, find directly, else respect the private
    // member `keep_case`
    return ignore_case ? freq.count(word) > 0 : has(word);
}

void Vocabulary::merge(const Vocabulary &other) {
    return merge(other.freq);
}

void Vocabulary::merge(const WordMap &freq) {
    for (const auto &it : freq) {
        add(it.first, it.second);
    }
    return;
}

void Vocabulary::sort(const std::string &order) {
    std::vector<WordFreq> vec(
        std::make_move_iterator(freq.begin()),
        std::make_move_iterator(freq.end()));

    if (order == "desc" || order == "asc") {
        auto compare = [&order](const WordFreq &w1, const WordFreq &w2) {
            return order == "desc" ? w1.second > w2.second
                                   : w1.second < w2.second;
            // TODO: by freq and alphabet
        };
        std::sort(vec.begin(), vec.end(), compare);

    } else if (order == "rand") {
        unsigned seed =
            std::chrono::system_clock::now().time_since_epoch().count();
        std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed));
    }

    return build(vec);
}

std::set<std::string> Vocabulary::words() const {
    std::set<std::string> keys;
    for (const auto &word : freq) {
        keys.insert(word.first);
    }
    return keys;
}

std::size_t Vocabulary::size() const {
    return freq.size();
}

bool Vocabulary::full() const {
    return freq.size() >= max_size;
}

void Vocabulary::clear() {
    freq.clear();
    itoa.clear();
    atoi.clear();
}

void Vocabulary::to_txt(const std::string &file) const {
    std::ofstream os(file);
    file::open(os, file);
    for (const auto &it : freq) {
        os << it.first << " " << it.second << std::endl;
    }
    os.close();

    std::ofstream os1, os2;
    file::open(os1, file + ".itoa");
    file::open(os2, file + ".atoi");
    for (const auto &it : itoa) {
        os1 << it.first << " " << it.second << std::endl;
        os2 << it.second << " " << it.first << std::endl;
    }
    os1.close();
    os2.close();
}

void Vocabulary::serialize(cereal::BinaryOutputArchive &archive) {
    archive(min_count, max_size, keep_case, freq, itoa, atoi);
}

void Vocabulary::serialize(cereal::BinaryInputArchive &archive) {
    archive(min_count, max_size, keep_case, freq, itoa, atoi);
}

WordMap::iterator Vocabulary::begin() {
    return freq.begin();
}

WordMap::iterator Vocabulary::end() {
    return freq.end();
}

WordMap::const_iterator Vocabulary::cbegin() {
    return freq.cbegin();
}

WordMap::const_iterator Vocabulary::cend() {
    return freq.cend();
}

std::string Vocabulary::operator[](const std::size_t &i) const {
    return itoa.at(i);
}
std::size_t Vocabulary::operator[](const std::string &w) const {
    return atoi.at(w);
}

std::ostream &operator<<(std::ostream &os, const Vocabulary &vocab) {
    os << "freq:" << std::endl;
    for (const auto &it : vocab.freq) {
        os << it.first << " " << it.second << std::endl;
    }
    os << "itoa:" << std::endl;
    for (const auto &it : vocab.itoa) {
        os << it.first << " " << it.second << std::endl;
    }
    os << "atoi:" << std::endl;
    for (const auto &it : vocab.atoi) {
        os << it.first << " " << it.second << std::endl;
    }

    return os;
}
