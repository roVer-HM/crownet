/*
 * Cell.cc
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include <math.h>
#include <algorithm>
#include <vector>

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

RegularCell::entry_t_ptr LocalSelector::applyTo(const RegularCell& cell) const {
    // to check local exists.... raise error.....
    auto val = cell.getLocal();
    return val;
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

RegularCell::entry_t_ptr MedianVisitor::applyTo(const RegularCell& cell) const {
  std::vector<double> count;
  for (const auto& e : cell.validIter()) {
      count.push_back(e.second->getCount());
  }
  auto m  = count.begin() + count.size()/2;
  int left = (int)(count.size()-1)/2; // lower-right
  int right = (int)count.size()/2;    // upper-left
  std::nth_element(count.begin(), m, count.end()); // only sort half of the data

  auto entry = cell.createEntry((double)(count[left] + count[right])/2);
  entry->touch(this->time);
  return entry;
}

RegularCell::entry_t_ptr InvSourceDistVisitor::applyTo(const RegularCell& cell) const {
  double itemSum = 0;
  double weightSum = 0;
  for (const auto& e : cell.validIter()) {
      EntryDist dist = e.second->getEntryDist();
      double w = 1.0 / std::max(1.0, dist.sourceEntry);
      weightSum += w;
      itemSum += w * e.second->getCount();
  }
  auto entry = cell.createEntry(itemSum/weightSum);
  entry->touch(this->time);
  return entry;
}

RegularCell::entry_t_ptr MaxCountVisitor::applyTo(const RegularCell& cell) const {
  double count = 0.0;
  for (const auto& e : cell) {
    count = std::max(count, e.second->getCount());
  }

  auto entry = cell.createEntry(count);
  entry->touch(this->time);
  return entry;
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
