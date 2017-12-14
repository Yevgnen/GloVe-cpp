#include "cooccur.h"
#include <armadillo>
#include <fstream>
#include <iostream>
#include <map>
#include "util.h"

arma::SpMat<double> CoMatrixBuilder::build(
    const std::string& file,
    const Vocabulary& vocab,
    unsigned long window,
    bool symmetric) {
    // Possible improvement to this function
    // 1. Use a better way to pass the
    // input stream since each single line could be very long leading to time
    // consumed string split.
    // 2. Don't use a sparse matrix but plain unordered_map only (with a proper
    // hash function).
    // 3. Use list/vector of (i, j, v) triplets instead of any maps.

    std::size_t vocab_size = vocab.size();
    arma::SpMat<double> cooccur(vocab_size, vocab_size);

    std::ifstream ifs;
    file::open(ifs, file);

    std::string line;
    unsigned long ix, iy;
    while (getline(ifs, line)) {
        std::vector<std::string>&& words = split(line);
        std::string center, context;
        for (std::size_t i = 0; i != words.size(); ++i) {
            center = words[i];
            if (!vocab.has(center)) {
                continue;
            }
            ix = vocab[center];

            for (std::size_t j = std::max<double>(i - window, 0); j != i; ++j) {
                context = words[j];
                if (!vocab.has(context)) {
                    continue;
                }
                iy = vocab[context];

                double weight = 1.0 / (i - j);
                cooccur(ix, iy) += weight;

                if (symmetric) {
                    cooccur(iy, ix) += weight;
                }
            }
        }
    }
    ifs.close();

    return cooccur;
}
