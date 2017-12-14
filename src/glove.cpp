#include "glove.h"
#include <cereal/archives/binary.hpp>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include "serialization.h"
#include "util.h"

GloVe::GloVe(
    std::size_t vocab_size,
    unsigned long size,
    double scale,
    double alpha,
    double threshold)
    : vocab_size(vocab_size), size(size), alpha(alpha), threshold(threshold) {
    // Params
    W1 = arma::randn(vocab_size, size);
    W2 = arma::randn(vocab_size, size);
    b1 = arma::zeros(vocab_size);
    b2 = arma::zeros(vocab_size);
    W1 *= scale;
    W2 *= scale;

    // Gradients
    dW1 = arma::zeros(vocab_size, size);
    dW2 = arma::zeros(vocab_size, size);
    db1 = arma::zeros(vocab_size);
    db2 = arma::zeros(vocab_size);

    // Gradient history
    GW1 = arma::zeros(arma::size(W1));
    GW2 = arma::zeros(arma::size(W2));
    Gb1 = arma::zeros(arma::size(b1));
    Gb2 = arma::zeros(arma::size(b1));
}

void GloVe::train(
    const arma::sp_mat& cooccur,
    unsigned long epochs,
    double lr,
    unsigned long threads,
    const std::string& logdir,
    unsigned long init_epoch) {
    unsigned long num_per_thread = std::ceil(cooccur.n_nonzero / threads);

    if (cooccur.n_rows != vocab_size || cooccur.n_cols != vocab_size) {
        throw std::runtime_error(
            "Size of coccurence matrix does not match vocabulary size");
    }

    std::vector<std::thread> vec_threads(threads);
    std::vector<double> partial_loss(threads);
    for (unsigned long epoch = init_epoch; epoch != epochs; ++epoch) {
        Timer timer;
        timer.start();
        vec_threads.clear();
        arma::sp_mat::const_iterator iter = cooccur.begin(), begin, end;
        for (std::size_t i = 0; i != threads; ++i) {
            begin = iter;
            end =
                (i != threads - 1 ? std::next(begin, num_per_thread + 1)
                                  : cooccur.end());
            vec_threads.emplace_back(
                &GloVe::train_thread, this, std::ref(cooccur), begin, end,
                std::ref(partial_loss[i]), lr);
            iter = end;
        }

        std::for_each(
            vec_threads.begin(), vec_threads.end(),
            [](std::thread& t) { return t.join(); });

        double loss =
            std::accumulate(partial_loss.begin(), partial_loss.end(), 0.0) /
            cooccur.n_nonzero;

        timer.stop();
        std::cout << std::fixed << "Epoch " << std::setw(3) << epoch
                  << " (took:  " << std::setprecision(3) << timer.elapsed()
                  << "s): Loss: " << std::setprecision(6) << loss << std::endl;

        std::string chkpt = "glove." + std::to_string(vocab_size) + "." +
                            std::to_string(size) + "." + std::to_string(epoch);

        BinaryArchiver::save(logdir + chkpt, *this);
    }
    return;
}

double GloVe::train_thread(
    const arma::sp_mat& cooccur,
    arma::sp_mat::const_iterator begin,
    arma::sp_mat::const_iterator end,
    double& loss,
    double lr) {
    loss = 0.0;

    arma::rowvec dw1 = arma::zeros<arma::rowvec>(size);
    arma::rowvec dw2 = arma::zeros<arma::rowvec>(size);
    double db1 = 0;
    double db2 = 0;

    double value, weight, sigma, l;
    for (auto iter = begin; iter != end; ++iter) {
        arma::uword i = iter.row(), j = iter.col();
        value = *iter;
        weight = weighted(value);
        sigma = difference(W1.row(i), W2.row(j), b1(i), b2(j), std::log(value));
        l = 0.5 * weight * std::pow(sigma, 2);
        loss += l;
        sigma *= weight;

        dw1 = sigma * W2.row(j);
        dw2 = sigma * W1.row(i);
        db1 = sigma;
        db2 = sigma;

#ifndef NDEBUG
        // Check gradient will failed in a multiple threads setting
        bool success = check_gradient(
            value, l, W1.row(i), W2.row(j), b1(i), b2(j), dw1, dw2, db1, db2);
        if (!success) {
            std::cerr << "Gradient check failed." << std::endl;
        }
#endif

        GW1.row(i) += arma::square(dw1);
        GW2.row(j) += arma::square(dw2);
        Gb1(i) += db1 * db1;
        Gb2(j) += db2 * db2;

        W1.row(i) -= lr * dw1 / arma::sqrt(GW1.row(i) + 1e-8);
        W2.row(j) -= lr * dw2 / arma::sqrt(GW2.row(j) + 1e-8);
        b1(i) -= lr * db1 / std::sqrt(Gb1(i) + 1e-8);
        b2(j) -= lr * db2 / std::sqrt(Gb2(j) + 1e-8);
    }

    return loss;
}

inline double GloVe::difference(
    const arma::rowvec& w1,
    const arma::rowvec& w2,
    double b1,
    double b2,
    double gold) const {
    return arma::as_scalar(arma::dot(w1, w2)) + b1 + b2 - gold;
}

inline double GloVe::weighted(double cooccur) const {
    return std::min(std::pow(cooccur / threshold, alpha), 1.0);
}

inline double GloVe::single_loss(
    double cooccur,
    const arma::rowvec& w1,
    const arma::rowvec& w2,
    double b1,
    double b2) const {
    return 0.5 * weighted(cooccur) *
           std::pow(difference(w1, w2, b1, b2, std::log(cooccur)), 2);
}

bool GloVe::check_gradient(
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
    double eps) const {
    arma::rowvec ndW1(arma::size(w1));
    arma::rowvec ndW2(arma::size(w2));
    double ndb1, ndb2;
    double y1, y2;

    // Check weights
    auto u(w1);
    auto v(w2);
    for (arma::uword i = 0; i != w1.n_rows; ++i) {
        for (arma::uword j = 0; j != w1.n_cols; ++j) {
            u(i, j) -= eps;
            y1 = single_loss(cooccur, u, w2, b1, b2);
            u(i, j) += 2 * eps;
            y2 = single_loss(cooccur, u, w2, b1, b2);
            ndW1(i, j) = 0.5 * (y2 - y1) / eps;
            u(i, j) -= eps;

            v(i, j) -= eps;
            y1 = single_loss(cooccur, w1, v, b1, b2);
            v(i, j) += 2 * eps;
            y2 = single_loss(cooccur, w1, v, b1, b2);
            ndW2(i, j) = 0.5 * (y2 - y1) / eps;
            v(i, j) -= eps;
        }
    }

    // Check biases
    y1 = single_loss(cooccur, w1, w2, b1 - eps, b2);
    y2 = single_loss(cooccur, w1, w2, b1 + eps, b2);
    ndb1 = 0.5 * (y2 - y1) / eps;

    y1 = single_loss(cooccur, w1, w2, b1, b2 - eps);
    y2 = single_loss(cooccur, w1, w2, b1, b2 + eps);
    ndb2 = 0.5 * (y2 - y1) / eps;

    return std::max({arma::max(arma::abs(dw1 - ndW1)),
                     arma::max(arma::abs(dw2 - ndW2)), std::abs(db1 - ndb1),
                     std::abs(db2 - ndb2)}) <= eps;
}

AnalogyPairs GloVe::most_similary(
    const std::string& word, unsigned long num, const Vocabulary& vocab) const {
    num = std::min(std::size_t(num), vocab.size());

    arma::rowvec key = W1.row(vocab[word]);
    arma::vec topk = W1 * key.t() / arma::sqrt(arma::sum(arma::square(W1), 1)) /
                     arma::norm(key);
    arma::uvec indices = arma::sort_index(topk, "descend");

    AnalogyPairs words;
    std::transform(
        indices.begin(), indices.begin() + num, std::back_inserter(words),
        [&vocab, &topk](const arma::uword& i) {
            return std::make_pair(vocab[i], topk(i));
        });

    return words;
}

void GloVe::to_txt(const std::string& file, const Vocabulary& v) const {
    std::string meta = file + ".meta";
    std::string center = file + ".w1";
    std::string context = file + ".w2";

    std::ofstream os;
    file::open(os, meta);
    os << size << std::endl;
    os.close();

    file::open(os, center);
    for (std::size_t i = 0; i != W1.n_rows; ++i) {
        os << v[i] << " ";
        W1.row(i).raw_print(os);
    }
    os.close();

    file::open(os, context);
    for (std::size_t i = 0; i != W2.n_rows; ++i) {
        os << v[i] << " ";
        W2.row(i).raw_print(os);
    }
    os.close();
}

void GloVe::serialize(cereal::BinaryOutputArchive& archive) {
    archive(
        vocab_size, size, alpha, threshold, W1, W2, b1, b2, dW1, dW2, db1, db2,
        GW1, GW2, Gb1, Gb2);
}

void GloVe::serialize(cereal::BinaryInputArchive& archive) {
    archive(
        vocab_size, size, alpha, threshold, W1, W2, b1, b2, dW1, dW2, db1, db2,
        GW1, GW2, Gb1, Gb2);
}
