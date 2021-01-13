/*
 * Cell.cc
 *
 *  Created on: Nov 23, 2020
 *      Author: sts
 */

#include "rover/dcd/regularGrid/RegularCell.h"
namespace rover {

std::ostream& operator<<(std::ostream& os, const RegularCell::value_type& i) {
  return os << "[" << i.first << ", " << i.second->str() << "]";
}

}  // namespace rover
