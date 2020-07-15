
#include "rover/aid/AidHeader_m.h"

namespace rover {

std::string AidHeader::str() const {
  std::ostringstream os;
  os << getClassName() << ", length = " << getChunkLength();
  return os.str();
}

}  // namespace rover
