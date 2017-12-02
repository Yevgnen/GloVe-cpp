#include <args.hxx>
#include <armadillo>
#include <iostream>
#include <vector>
#include "glove.h"
#include "serialization.h"
#include "vocabulary.h"

int main(int argc, char** argv) {
    args::ArgumentParser parser(
        "GloVe: Global Vectors for Word Representation");
    args::HelpFlag help(
        parser, "help", "Display this help menu", {'h', "help"});
    args::ValueFlag<std::string> model(
        parser, "model", "Model checkpoint", {"model"},
        args::Options::Required);
    args::ValueFlag<unsigned long> size(
        parser, "size", "word vector size", {"size"}, 200);
    args::ValueFlag<std::string> vocab(
        parser, "vocab", "Vocabulary file", {"vocab"}, args::Options::Required);
    args::ValueFlag<std::string> word(
        parser, "word", "Word", {"word"}, args::Options::Required);
    args::ValueFlag<unsigned long> num(
        parser, "num", "Number of analogies", {"num"}, 10);

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

    // Build vocabulary
    std::cout << "Building vocabulary..." << std::endl;
    Vocabulary v = Vocabulary();
    BinaryArchiver::load(args::get(vocab), v);
    std::cout << "Vocab size: " << v.size() << std::endl;

    // Load GloVe model
    GloVe glove(v.size(), args::get(size));
    BinaryArchiver::load(args::get(model), glove);

    // Word analogies
    AnalogyPairs words =
        glove.most_similary(args::get(word), args::get(num), v);
    for (const auto& p : words) {
        std::cout << p.first << ": " << p.second << std::endl;
    }

    return 0;
}
