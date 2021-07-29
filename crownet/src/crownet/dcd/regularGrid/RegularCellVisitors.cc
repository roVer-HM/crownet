/*
 * Cell.cc
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularCellVisitors.h"

namespace crownet {

RegularCell::entry_t_ptr YmfVisitor::applyTo(const RegularCell& cell) const {
  RegularCell::entry_t_ptr ret = nullptr;
  for (const auto& e : cell.validIter()) {
    if (ret == nullptr) {
      ret = e.second;
      continue;
    }
    if (e.second->getMeasureTime() > ret->getMeasureTime()) {
      ret = e.second;
    }
  }
  return ret;
}


RegularCell::entry_t_ptr MeanVisitor::applyTo(const RegularCell& cell) const {
  double sum = 0;
  double num = 0;
  for (const auto& e : cell.validIter()) {
      ++num;
      sum += e.second->getCount();
  }
  auto entry = cell.createEntry(sum/num);
  entry->touch(this->time);
  return entry;
}

RegularCell::entry_t_ptr AlgBiggest::applyTo(const RegularCell& cell) const {
  RegularCell::value_type ret;
  for (const auto& e : cell) {
    if (!ret.second) ret.second = e.second;
    if (e.second->getCount() > ret.second->getCount()) {
      ret = std::make_pair(e.first, e.second);
    }
  }
  return ret.second;
}

RegularCell::entry_t_ptr AlgSmall::applyTo(const RegularCell& cell) const {
  RegularCell::value_type ret;
  for (const auto& e : cell) {
    if (!ret.second) ret.second = e.second;
    if (e.second->getCount() < ret.second->getCount()) {
      ret = std::make_pair(e.first, e.second);
    }
  }
  return ret.second;
}

}  // namespace crownet
