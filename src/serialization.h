#ifndef _SRC_SERIALIZATION_H_
#define _SRC_SERIALIZATION_H_

#include <armadillo>
#include <cereal/archives/binary.hpp>
#include "serialization.h"

namespace cereal {

template <class Archive, typename T>
void save(Archive& archive, arma::Mat<T> const& mat) {
    archive(cereal::binary_data(mat.memptr(), sizeof(T) * mat.size()));
}

template <class Archive, typename T>
void load(Archive& archive, arma::Mat<T>& mat) {
    T* ptr = new T[mat.size()];
    archive(cereal::binary_data(ptr, sizeof(T) * mat.size()));
    mat = arma::Mat<T>(ptr, mat.n_rows, mat.n_cols, false);
}

};  // namespace cereal

class BinaryArchiver {
public:
    template <typename T>
    static void save(const std::string& file, const T& obj) {
        std::ofstream os(file, std::ios::binary);
        if (os) {
            cereal::BinaryOutputArchive archive(os);
            archive(obj);
        }
        os.close();
    }

    template <typename T>
    static void load(const std::string& file, T& obj) {
        std::ifstream is(file, std::ios::binary);
        if (is) {
            cereal::BinaryInputArchive archive(is);
            archive(obj);
        }
        is.close();
    }
};

#endif /* _SRC_SERIALIZATION_H_ */
