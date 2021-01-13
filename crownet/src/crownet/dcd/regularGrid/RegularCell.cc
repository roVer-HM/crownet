/*
 * Cell.cc
 *
 *  Created on: Nov 23, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularCell.h"
namespace crownet {

std::ostream& operator<<(std::ostream& os, const RegularCell::value_type& i) {
  return os << "[" << i.first << ", " << i.second->str() << "]";
}

}  // namespace crownet
