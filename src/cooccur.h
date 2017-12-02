#ifndef _SRC_COOCCUR_H_
#define _SRC_COOCCUR_H_

#include <armadillo>
#include "vocabulary.h"

class CoMatrixBuilder {
public:
    CoMatrixBuilder() = default;

    static arma::SpMat<double> build(
        const std::string& file,
        const Vocabulary& vocab,
        unsigned long window = 10,
        bool symmetric = true);
};

#endif /* _SRC_COOCCUR_H_ */
