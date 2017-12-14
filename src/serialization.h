#ifndef _SRC_SERIALIZATION_H_
#define _SRC_SERIALIZATION_H_

#include <armadillo>
#include <cereal/archives/binary.hpp>
#include <stdexcept>
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
        std::ofstream os;
        auto old_state = os.exceptions();
        try {
            os.exceptions(std::ios::badbit | std::ios::failbit);
            os.open(file, std::ios::binary);
        } catch (const std::ios::failure& e) {
            throw std::runtime_error("failed to open binary file: " + file);
        }
        os.exceptions(old_state);
        cereal::BinaryOutputArchive archive(os);
        archive(obj);
    }

    template <typename T>
    static void load(const std::string& file, T& obj) {
        std::ifstream is;
        auto old_state = is.exceptions();
        try {
            is.exceptions(std::ios::badbit | std::ios::failbit);
            is.open(file, std::ios::binary);
        } catch (const std::ios::failure& e) {
            throw std::runtime_error("failed to open binary file: " + file);
        }
        is.exceptions(old_state);
        cereal::BinaryInputArchive archive(is);
        archive(obj);
    }
};

#endif /* _SRC_SERIALIZATION_H_ */
