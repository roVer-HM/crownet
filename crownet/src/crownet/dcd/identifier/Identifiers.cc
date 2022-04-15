/*
 * Identifiers.cc
 *
 *  Created on: Nov 23, 2020
 *      Author: sts
 */

#include "crownet/dcd/identifier/Identifiers.h"

namespace crownet {

std::ostream& operator<<(std::ostream& os, const CellIdentifiere& i) {
  return os << i.str();
}

std::string GridCellID::str() const {
  std::stringstream os;
  os << "[" << id.first << "," << id.second << "]";
  return os.str();
}

bool GridCellID::operator<(const GridCellID& rhs) const {
  return id.first < rhs.id.first ||
         (!(rhs.id.first < id.first) && id.second < rhs.id.second);
}

bool GridCellID::operator==(const GridCellID& rhs) const {
  return id.first == rhs.id.first && id.second == rhs.id.second;
}
bool GridCellID::operator!=(const GridCellID& rhs) const {
  return !(*this == rhs);
}

int GridCellID::columns() const { return 2; }
void GridCellID::writeTo(std::ostream& out, const std::string& sep) const {
  out << id.first << sep << id.second;
}

void GridCellID::writeHeaderTo(std::ostream& out,
                               const std::string& sep) const {
  out << "Grid_x" << sep << "Grid_y";
}

int IntIdentifer::columns() const { return 1; }
void IntIdentifer::writeTo(std::ostream& out, const std::string& sep) const {
  out << id;
}
void IntIdentifer::writeHeaderTo(std::ostream& out,
                                 const std::string& sep) const {
  out << "IntIdentifier";
}

//
// std::ostream& operator<<(std::ostream& os, const NodeIdentifiere& i) {
//  return os << i.str();
//}

}  // namespace crownet
