#ifndef _SRC_COOCCUR_H_
#define _SRC_COOCCUR_H_

#include <list>
#include "vocabulary.h"

struct CoRec {
    unsigned long i;
    unsigned long j;
    double weight;

    CoRec() = delete;
    CoRec(unsigned long i, unsigned long j, double weight)
        : i(i), j(j), weight(weight) {}
    CoRec(const CoRec& o) : i(o.i), j(o.j), weight(o.weight) {}
    CoRec(CoRec&& o) : i(o.i), j(o.j), weight(o.weight) {}

    CoRec& operator=(const CoRec& x) {
        if (this == &x) {
            return *this;
        }
        i = x.i;
        j = x.j;
        weight = x.weight;
        return *this;
    }

    CoRec& operator+=(const CoRec& x) {
        if (i == x.i && j == x.j) {
            weight += x.weight;
        }
        return *this;
    }
    bool operator==(const CoRec& x) const {
        return i == x.i && j == x.j;
    }
    bool operator!=(const CoRec& x) const {
        return !((*this) == x);
    }
    bool operator<(const CoRec& x) const {
        return i < x.i ? true : (i == x.i && j < x.j);
    }
    bool operator<=(const CoRec& x) const {
        return (*this < x) || (*this == x);
    }
};

using CoRecs = std::list<CoRec>;

class CoMatrixBuilder {
public:
    CoMatrixBuilder() = delete;

    static CoRecs build(
        const std::string& file,
        const Vocabulary& vocab,
        unsigned long window = 10,
        bool symmetric = true,
        unsigned long threshold = 5000 * 5000,
        bool shuffle = true);
};

#endif /* _SRC_COOCCUR_H_ */
