#pragma once

#include <string>
#include <sstream>

namespace jian {

template<typename T, typename U>
T lexical_cast(U && u) {
    std::stringstream stream;
    T t;

    stream << u;
    stream >> t;
    return t;
}

}

