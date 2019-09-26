#pragma once

#include <ostream>
#include <spdlog/fmt/ostr.h>

struct AudioConfigData {
    int samples_ = -1;
    int device_ = -1;
};

inline std::ostream & operator << (std::ostream &out, const AudioConfigData &x) {
    return out << "samples = " << x.samples_ << ", device = " << x.device_;
}
