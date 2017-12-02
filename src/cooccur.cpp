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
    std::size_t vocab_size = vocab.size();

    std::map<std::tuple<std::size_t, std::size_t>, double> triples;
    std::ifstream stream(file);
    std::string line;

    if (stream) {
        while (getline(stream, line)) {
            std::vector<std::string>&& words = split(line);
            std::string center, context;
            for (std::size_t i = 0; i != words.size(); ++i) {
                center = words[i];
                if (!vocab.has(center)) {
                    continue;
                }

                for (std::size_t j = std::max<double>(i - window, 0); j != i;
                     ++j) {
                    context = words[j];
                    if (!vocab.has(context)) {
                        continue;
                    }

                    double weight = 1.0 / (i - j);
                    triples[std::tuple<std::size_t, std::size_t>(
                        vocab[words[i]], vocab[words[j]])] += weight;

                    if (symmetric) {
                        triples[std::tuple<std::size_t, std::size_t>(
                            vocab[words[j]], vocab[words[i]])] += weight;
                    }
                }
            }
        }
    }
    // TODO: handling exception
    stream.close();

    unsigned long num_values = triples.size();
    arma::umat indices = arma::zeros<arma::umat>(2, num_values);
    arma::vec values = arma::zeros<arma::vec>(num_values);

    std::size_t i = 0;
    for (const auto& t : triples) {
        indices(0, i) = std::get<0>(t.first);
        indices(1, i) = std::get<1>(t.first);
        values(i) = t.second;
        ++i;
    }

    arma::SpMat<double> cooccur(indices, values);

    return cooccur;
}
