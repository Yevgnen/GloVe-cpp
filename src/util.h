#ifndef _SRC_UTIL_H_
#define _SRC_UTIL_H_

#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <vector>

std::vector<std::string> split(
    const std::string &str, const char &delimiter = ' ');

std::string join(
    const std::vector<std::string> &strs, const char &delimiter = 0);
std::string join(
    std::initializer_list<std::string> lst, const char &delimiter = 0);

std::string lower(const std::string &str);

namespace path {

std::string join(
    const std::vector<std::string> &strs, const char &delimiter = '/');
std::string join(
    std::initializer_list<std::string> lst, const char &delimiter = '/');

}  // namespace path

class Timer {
public:
    Timer() = default;
    std::chrono::steady_clock::time_point now() const {
        return std::chrono::steady_clock::now();
    }
    std::chrono::steady_clock::time_point start() {
        return start_time = std::chrono::steady_clock::now();
    }
    std::chrono::steady_clock::time_point stop() {
        return end_time = std::chrono::steady_clock::now();
    }

    template <class T = double>
    T elapsed() const {
        return std::chrono::duration_cast<std::chrono::duration<T>>(
                   end_time - start_time)
            .count();
    }

private:
    std::chrono::steady_clock::time_point start_time;
    std::chrono::steady_clock::time_point end_time;
};

#endif /* _SRC_UTIL_H_ */
