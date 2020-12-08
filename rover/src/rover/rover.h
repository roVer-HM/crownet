#pragma once

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/Geometry_m.h"
#include "rover/common/util/RingBuffer.h"

namespace std {
std::ostream &operator<<(std::ostream &os, const std::pair<int, int> &pair);

inline std::ostream &operator<<(std::ostream &os,
                                const std::pair<int, int> &pair) {
  return os << "[ " << pair.first << ", " << pair.second << "]";
}
}  // namespace std
