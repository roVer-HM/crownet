
#include "crownet/aid/AidHeader_m.h"

namespace crownet {

std::string AidHeader::str() const {
  std::ostringstream os;
  os << getClassName() << ", length = " << getChunkLength();
  return os.str();
}

}  // namespace crownet
