#ifndef _SRC_GLOVE_H_
#define _SRC_GLOVE_H_

#include <armadillo>
#include <cereal/archives/binary.hpp>
#include <iostream>
#include <vector>
#include "cooccur.h"
#include "serialization.h"
#include "vocabulary.h"

using AnalogyPair = std::pair<std::string, double>;
using AnalogyPairs = std::vector<std::pair<std::string, double>>;

class GloVe {
public:
    GloVe() = default;
    explicit GloVe(
        std::size_t vocab_size,
        unsigned long size = 200,
        double init_scale = 1e-3,
        double alpha = 3.0 / 4,
        double threshold = 100);

    void train(
        const CoRecs& cooccur,
        unsigned long epoch = 10,
        double lr = 1e-3,
        unsigned long threads = 12,
        const std::string& logdir = "./",
        unsigned long init_epoch = 0);
    double train_thread(
        const CoRecs& cooccur,
        CoRecs::const_iterator begin,
        CoRecs::const_iterator end,
        double& loss,
        double lr);
    inline double difference(
        const arma::rowvec& w1,
        const arma::rowvec& w2,
        double b1,
        double b2,
        double gold) const;
    inline double weighted(double cooccur) const;
    inline double single_loss(
        double cooccur,
        const arma::rowvec& w1,
        const arma::rowvec& w2,
        double b1,
        double b2) const;

    bool check_gradient(
        double cooccur,
        double loss,
        const arma::rowvec& w1,
        const arma::rowvec& w2,
        double b1,
        double b2,
        const arma::rowvec& dw1,
        const arma::rowvec& dw2,
        double db1,
        double db2,
        double eps = 1e-7) const;

    AnalogyPairs most_similary(
        const std::string& word,
        unsigned long num,
        const Vocabulary& vocab) const;

    void to_txt(const std::string& file, const Vocabulary& vocab) const;

    void serialize(cereal::BinaryOutputArchive& archive);
    void serialize(cereal::BinaryInputArchive& archive);

private:
    std::size_t vocab_size;
    unsigned long size;
    double alpha;
    double threshold;
    arma::mat W1;
    arma::mat W2;
    arma::colvec b1;
    arma::colvec b2;
    arma::mat dW1;
    arma::mat dW2;
    arma::colvec db1;
    arma::colvec db2;
    arma::mat GW1;
    arma::mat GW2;
    arma::mat Gb1;
    arma::mat Gb2;
};

#endif /* _SRC_GLOVE_H_ */
