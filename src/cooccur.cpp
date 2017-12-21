#include "cooccur.h"
#include <deque>
#include <fstream>
#include <iostream>
#include "util.h"

CoRecs CoMatrixBuilder::build(
    const std::string& file,
    const Vocabulary& vocab,
    unsigned long window,
    bool symmetric,
    unsigned long threshold,
    bool shuffle) {
    // Check whether a word is OOV
    auto lookup = [&](const std::string& key, unsigned long& id) {
        try {
            id = vocab[key];
        } catch (const std::out_of_range& e) {
            return 0;
        }
        return 1;
    };

    // Build an auxiliary lookup table
    std::size_t vsize = vocab.size();
    std::vector<unsigned long> index(vsize + 1, 0);
    index[0] = 0;
    for (std::size_t i = 1; i != vsize + 1; ++i) {
        index[i] = index[i - 1] + std::min(threshold / i, vsize);
    }
    std::vector<double> bigram_table(index[vsize], 0);

    // Open corpus file and ready to read
    std::ifstream is;
    file::open(is, file);
    std::string word, center, context;

    // Context and its availability
    std::deque<unsigned long> ids;
    std::deque<bool> flags;

    // Center and its availability
    unsigned long id;
    bool exists;

    // Put the first word of the first line into context
    is >> context;
    exists = lookup(context, id);
    ids.push_back(exists ? id : 0);
    flags.push_back(exists);

    std::list<CoRec> low_cooccur;  // TODO: split into chunks
    while (!is.eof()) {
        is >> center;
        // If this line has only 1 word, skip
        if (center.empty()) {
            continue;
        }

        exists = lookup(center, id);
        if (exists) {
            for (std::size_t i = 0; i != ids.size(); ++i) {
                // If context word is OOV, skip
                if (!flags[i]) {
                    continue;
                }

                double weight = 1.0 / (ids.size() - i);
                bool flag = (threshold / (id + 1) >= (ids[i] + 1));
                if (flag) {
                    bigram_table[index[id] + ids[i]] += weight;
                } else {
                    low_cooccur.emplace_back(id, ids[i], weight);
                }

                if (symmetric) {
                    if (flag) {
                        bigram_table[index[ids[i]] + id + 1] += weight;
                    } else {
                        low_cooccur.emplace_back(ids[i], id, weight);
                    }
                }
            }
        }

        // If line breaks, separate context between paragraphs
        if (is.peek() == '\n') {
            ids.clear();
            flags.clear();
            is >> context;
            exists = lookup(context, id);
            ids.push_back(exists ? id : 0);
            flags.push_back(exists);
            center = "";
        } else {
            // Remove the oldest history
            if (ids.size() >= window) {
                ids.pop_front();
                flags.pop_front();
            }
            // Current center word becomes history
            ids.push_back(id);
            flags.push_back(exists);
        }
    }
    is.close();

    // Turn the bigram records into `CoRec` records
    std::list<CoRec> high_cooccur;
    for (std::size_t i = 0; i != vsize; ++i) {
        for (std::size_t j = 0; j != index[i + 1] - index[i]; ++j) {
            double weight = bigram_table[index[i] + j];
            if (weight > 0) {
                high_cooccur.emplace_back(i, j, weight);
            }
        }
    }

    // Merge sorted high and low `CoRec`s
    // TODO: Merge and aggregate at the same time
    low_cooccur.sort();
    high_cooccur.merge(low_cooccur);

    // Aggregate duplicate records
    std::list<CoRec> cooccur;
    cooccur.push_back(std::move(high_cooccur.front()));
    high_cooccur.pop_front();
    while (!high_cooccur.empty()) {
        CoRec current = std::move(high_cooccur.front());
        high_cooccur.pop_front();
        if (current == cooccur.back()) {
            cooccur.back() += current;
        } else {
            cooccur.push_back(current);
        }
    }

#ifndef NDEBUG
    // To check whether all the co-occurrence records are sorted by id
    for (auto iter = cooccur.begin(); std::next(iter) != cooccur.end();
         ++iter) {
        auto next = std::next(iter);
        if ((*next) <= (*iter)) {
            std::cerr << "found unmerged or unsorted co-occurence record: ("
                      << iter->i << ", " << iter->j << ", " << iter->weight
                      << ") <->"
                      << "(" << next->i << ", " << next->j << ", "
                      << next->weight << ")" << std::endl;
        }
    }
#endif

    if (shuffle) {
        std::vector<CoRec> vec_records(
            std::make_move_iterator(cooccur.begin()),
            std::make_move_iterator(cooccur.end()));
        std::shuffle(
            vec_records.begin(), vec_records.end(),
            std::mt19937(std::random_device()()));
        std::copy(
            std::make_move_iterator(vec_records.begin()),
            std::make_move_iterator(vec_records.end()), cooccur.begin());
    }

    return cooccur;
}
