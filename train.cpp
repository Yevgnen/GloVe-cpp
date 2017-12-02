#include <args.hxx>
#include <armadillo>
#include <iostream>
#include <vector>
#include "cooccur.h"
#include "glove.h"
#include "serialization.h"
#include "util.h"
#include "vocabulary.h"

int main(int argc, char** argv) {
    args::ArgumentParser parser(
        "GloVe: Global Vectors for Word Representation");
    args::HelpFlag help(
        parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> input(
        parser, "input", "Corpus file", {"input"}, args::Options::Required);
    args::ValueFlag<std::string> model(
        parser, "model", "GloVe model", {"model"});
    args::ValueFlag<std::string> logdir(
        parser, "logdir", "Log directory", {"logdir", "log"},
        args::Options::Required);
    args::ValueFlag<CountType> vocab_size(
        parser, "vocab_size", "Max vocabulary size", {"vocab-size"}, 1e7);
    args::ValueFlag<unsigned long> min_count(
        parser, "min_count", "Word frequecy lower than which will be ignored",
        {"min-count"}, 1);
    args::Flag keep_case(
        parser, "keep_case", "Whether to keep case when build vocabulary",
        {"keep-case"}, false);
    args::ValueFlag<unsigned long> window(
        parser, "window", "Window size for building cooccur matrix", {"window"},
        10);
    args::Flag symmetric(
        parser, "symmetric", "Whether to use symmetric window", {"symmetric"},
        true);
    args::ValueFlag<unsigned long> size(
        parser, "size", "Word vector size", {"size"}, 200);
    args::ValueFlag<double> threshold(
        parser, "threshold", "Cooccurence threshold for weighting function",
        {"threshold"}, 100);
    args::ValueFlag<unsigned long> epoch(
        parser, "epochs", "Number of total epochs", {"epochs"}, 10);
    args::ValueFlag<double> lr(
        parser, "lr", "Learning rate", {"lr", "learning-rate"}, 1e-3);
    args::ValueFlag<unsigned long> threads(
        parser, "threads", "Number of threads to use", {"threads"}, 12);
    args::ValueFlag<unsigned long> seed(
        parser, "seed", "Random seed (should > 0)", {"seed"});

    try {
        parser.ParseCLI(argc, argv);
    } catch (args::Help) {
        std::cout << parser;
        return 0;
    } catch (args::ParseError e) {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    } catch (args::RequiredError e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (seed) {
        arma::arma_rng::set_seed(args::get(seed));
    } else {
        arma::arma_rng::set_seed_random();
    }

    // Build vocabulary
    std::cout << "Building vocabulary..." << std::endl;
    Vocabulary v = Vocabulary(
        args::get(min_count), args::get(vocab_size), args::get(keep_case));
    v.build(args::get(input));
    BinaryArchiver::save(path::join({args::get(logdir), "vocab.bin"}), v);
    std::cout << "Vocab size: " << v.size() << std::endl;

    // Build Co-occurrence matrix
    std::cout << "Building co-occurrence matrix..." << std::endl;
    arma::sp_mat&& co = CoMatrixBuilder::build(
        args::get(input), v, args::get(window), args::get(symmetric));
    std::cout << "Nonzero elements: " << co.n_nonzero << std::endl;

    // Train
    std::cout << "Training..." << std::endl;
    unsigned long init_epoch = 0;
    GloVe glove(v.size(), args::get(size), 1e-3, 0.75, args::get(threshold));
    if (model) {
        BinaryArchiver::load(args::get(model), glove);
        init_epoch = std::stol(split(args::get(model), '.').at(3)) + 1;
    }
    glove.train(
        co, args::get(epoch), args::get(lr), args::get(threads),
        args::get(logdir), init_epoch);
    glove.to_txt(path::join({args::get(logdir), "wordvec.txt"}), v);

    return 0;
}
