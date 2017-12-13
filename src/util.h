#ifndef _SRC_UTIL_H_
#define _SRC_UTIL_H_

#include <algorithm>
#include <chrono>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

std::string lower(const std::string &str);

std::vector<std::string> split(
    const std::string &str, const char &delimiter = ' ');

template <typename Arg, typename... Args>
std::string join(const char &delimiter, const Arg &arg, const Args &... args) {
    if (!sizeof...(args)) {
        return arg;
    }

    std::ostringstream os;
    os << arg;
    static_cast<void>(
        std::initializer_list<int>{(os << delimiter << args, 0)...});

    return os.str();
}

std::string trim(
    const std::string &s,
    const char &delimiter = ' ',
    bool left = true,
    bool right = true);
std::string trim_left(const std::string &s, const char &delimiter = ' ');
std::string trim_right(const std::string &s, const char &delimiter = ' ');

namespace path {

std::string normalize(const std::string &filepath, const char &delimiter);

template <typename Arg, typename... Args>
std::string join(Arg &&arg, Args &&... args) {
    const char &delimiter = '/';

    return ::join(
        delimiter, trim_right(std::forward<Arg>(arg), delimiter),
        trim(std::forward<Args>(args), delimiter)...);
}

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
