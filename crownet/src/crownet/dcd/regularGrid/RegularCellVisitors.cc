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
#include <omnetpp.h>

namespace crownet {


void IncrementerVisitor::applyTo(RegularCell& cell) {
  for (auto e : cell) {
    e.second->incrementCount(time);
  }
}


void ResetVisitor::applyTo(RegularCell& cell) {
  for (auto e : cell) {
    e.second->reset(time);
  }
}

void ResetLocalVisitor::applyTo(RegularCell& cell) {
    if (cell.hasLocal()) {
      cell.getLocal()->reset(time);
    }
  }


void ClearVisitor::applyTo(RegularCell& cell) {
  for (auto e : cell) {
    e.second->clear(time);
  }
}

void TTLCellAgeHandler::applyTo(RegularCell& cell) {
    if (checkLastCall()){ // only execute once
        for (auto e : cell) {
            if (ttl > 0.0 && time > (ttl + e.second->getMeasureTime())){
                if (e.first == dcdMap->getOwnerId()){
                    // if `e` is the local map and this value reached the TTL, it is
                    // necessary to clear the Node->Cell association map of the DcDMap
                    // where `Cell` equals to current cell.
                    dcdMap->removeFromNeighborhood(cell.getCellId());
                }
                e.second->reset(simtime_t::ZERO);
            }
        }
    }
}

void ClearSelection::applyTo(RegularCell& cell) {
  for (auto e : cell) {
    e.second->setSelectedIn("");
  }
}


void ClearLocalVisitor::applyTo(RegularCell& cell) {
  if (cell.hasLocal()) {
    cell.getLocal()->clear(getTime());
  }
}

void TtlClearLocalVisitor::applyTo(RegularCell& cell){
    auto ownerCell = getMap()->getOwnerCell();
    if (cell.hasLocal()) {
        auto e = cell.getLocal();
        if (e->getCount() > 0.0){
            // set entry to zero and update time (keep entry valid)
            cell.getLocal()->clear(getTime());
        } else {
            if (getTime() > (e->getMeasureTime() + getZeroTtl())){
                if (getKeepZeroDistance() > 0.0){
                    double dist = getMap()->getCellKeyProvider()->maxCellDist(ownerCell, cell.getCellId());
                    if (dist < getKeepZeroDistance()){
                        // keep  zero value because cell is very close to owner location
                        // so we can still assume that no one is in this cell
                        cell.getLocal()->touch(getTime());
                        return;
                    }
                }
                //invalidate entry
                e->reset(getTime());
            }
        }
    }
}

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
  if (ret){
      ret->setSelectionRank(0.0);
  }
  return ret;
}


YmfPlusDistVisitor::sum_data YmfPlusDistVisitor::getSums(const RegularCell& cell) const {
    double age_sum = 0.0;
    double age_min = std::numeric_limits<double>::max();
    double distance_sum = 0.0;
    double dist_min = std::numeric_limits<double>::max();
    double now = this->time.dbl(); // current time the visitor is called.

    double age = .0;
    double dist = .0;
    int count = 0;
    for (const auto& e : cell.validIter()) {
        /*
         * Collect the sum of age difference relative to the
         * youngest measure (i.e. smallest age). For this
         * sum up all ages and find the smallest age at the same
         * time. Subtract count*age_min after the fact from age_sum.
         */
        age = now - e.second->getMeasureTime().dbl();
        if(age < age_min){
            age_min = age;
        }
        age_sum += age;

        /*
         * In the case of a local entry the 'sourceEntry' and 'ownerEntry'
         * values must be the same. Thus the sum of all sourceEntry values
         * is correct.
         * TODO EntryDist for localEntry is ZERO {0, 0, 0}
         */
        dist = e.second->getEntryDist().sourceEntry;
        if (dist < dist_min){
            dist_min = dist;
        }
        distance_sum += dist;
        ++count;
    }
    // normalize over the age (min)difference
    age_sum = age_sum - count*age_min;
    // dist sum must not be updated because the some of the distance step function is
    // the correct normalization value.
    return sum_data{age_sum, age_min, distance_sum, dist_min, count};
}

RegularCell::entry_t_ptr YmfPlusDistVisitor::applyTo(const RegularCell& cell) const {

    sum_data d = getSums(cell);
    double now = this->time.dbl(); // current time the visitor is called.

    RegularCell::entry_t_ptr ret = nullptr;
    double ret_ymfPlusDist = 0.0;
    double ymfPlusDist = 0.0;
    for (const auto& e : cell.validIter()) {
      double age = (now - e.second->getMeasureTime().dbl()) - d.age_min;
      double dist = e.second->getEntryDist().sourceEntry;

      // set rank to maximum value (=1.0) if the sum equals to zero. (i.e. there is only one element in validIter)
      double ageRank = (d.age_sum == 0) ? 1.0 : age/d.age_sum;
      double distRank = (d.dist_sum == 0) ? 1.0 : dist/d.dist_sum;

      ymfPlusDist = alpha * ageRank + (1-alpha) * distRank;
      e.second->setSelectionRank(ymfPlusDist);

      if (ret == nullptr) {
        ret = e.second;
        ret_ymfPlusDist = ymfPlusDist;
        continue;
      }

      if (ymfPlusDist < ret_ymfPlusDist) {
        ret = e.second;
        ret_ymfPlusDist = ymfPlusDist;
      }
    }
    return ret;
}


RegularCell::entry_t_ptr YmfPlusDistStepVisitor::applyTo(const RegularCell& cell) const {

    sum_data d = getSums(cell);
    double now = this->time.dbl(); // current time the visitor is called.

    RegularCell::entry_t_ptr ret = nullptr;
    double ret_ymfPlusDist = 0.0;
    double ymfPlusDist = 0.0;
    for (const auto& e : cell.validIter()) {
      double age = (now - e.second->getMeasureTime().dbl()) - d.age_min;
      double dist = getDistValue(e.second->getEntryDist().sourceEntry);

      // set rank to maximum value (=1.0) if the sum equals to zero. (i.e. there is only one element in validIter)
      double ageRank = (d.age_sum == 0.0) ? 1.0 : (age)/d.age_sum; // 1.0 if one or all are the same (i.e all are the samllest)
      double distRank = ( d.dist_sum == 0.0) ? 1.0 : dist/d.dist_sum; // 1.0 if one value

      ymfPlusDist = alpha * ageRank + (1-alpha) * distRank;
      e.second->setSelectionRank(ymfPlusDist);
      if (ret == nullptr) {
        ret = e.second;
        ret_ymfPlusDist = ymfPlusDist;
        continue;
      }

      if (ymfPlusDist < ret_ymfPlusDist) {
        ret = e.second;
        ret_ymfPlusDist = ymfPlusDist;
      }
    }
    return ret;
}

const double YmfPlusDistStepVisitor::getDistValue(const double dist) const {
        return (dist <= stepDist) ? stepDist : dist;
}

YmfPlusDistVisitor::sum_data YmfPlusDistStepVisitor::getSums(const RegularCell& cell) const {
    double age_sum = 0.0;
    double age_min = std::numeric_limits<double>::max();
    double distance_sum = 0.0;
    double dist_min = std::numeric_limits<double>::max();
    double now = this->time.dbl(); // current time the visitor is called.

    double age = .0;
    double dist = .0;
    int count = 0;
    for (const auto& e : cell.validIter()) {
        /*
         * Collect the sum of age difference relative to the
         * youngest measure (i.e. smallest age). For this
         * sum up all ages and find the smallest age at the same
         * time. Subtract count*age_min after the fact from age_sum.
         */
        age = now - e.second->getMeasureTime().dbl();
        if(age < age_min){
            age_min = age;
        }
        age_sum += age;


        /*
         * sum up step function result not only distance
         */

        dist = getDistValue(e.second->getEntryDist().sourceEntry);
        if (dist < dist_min){
            dist_min = dist;
        }
        distance_sum += dist;
        ++count;
    }
    // normalize over the age (min)difference
    age_sum = age_sum - count*age_min;
    // dist sum must not be updated because the some of the distance step function is
    // the correct normalization value.
    return sum_data{age_sum, age_min, distance_sum, dist_min, count};
}

RegularCell::entry_t_ptr LocalSelector::applyTo(const RegularCell& cell) const {
    // to check local exists.... raise error.....
    auto val = cell.getLocal();
    val->setSelectionRank(0.0);
    return val;
}


RegularCell::entry_t_ptr MeanVisitor::applyTo(const RegularCell& cell) const {
  double sum = 0;
  double num = 0;
  double time = 0;
  for (const auto& e : cell.validIter()) {
      ++num;
      sum += e.second->getCount();
      time  += e.second->getMeasureTime().dbl(); // todo
  }
  auto entry = cell.createEntry(sum/num);
  entry->setMeasureTime(time/num);
  entry->setReceivedTime(time/num);
  entry->setSelectionRank(0.0);
  return entry;
}

RegularCell::entry_t_ptr MedianVisitor::applyTo(const RegularCell& cell) const {
  std::vector<double> count;
  std::vector<double> time;
  for (const auto& e : cell.validIter()) {
      count.push_back(e.second->getCount());
      time.push_back(e.second->getMeasureTime().dbl());
  }
  auto m  = count.begin() + count.size()/2;
  int left = (int)(count.size()-1)/2; // lower-right
  int right = (int)count.size()/2;    // upper-left
  std::nth_element(count.begin(), m, count.end()); // only sort half of the data
  std::nth_element(time.begin(), m, time.end()); // only sort half of the data


  auto entry = cell.createEntry((double)(count[left] + count[right])/2);
  entry->setMeasureTime((double)(count[left] + count[right])/2);
  entry->setReceivedTime((double)(time[left] + time[right])/2);
  entry->setSelectionRank(0.0);
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
  entry->setSelectionRank(0.0);
  return entry;
}

RegularCell::entry_t_ptr MaxCountVisitor::applyTo(const RegularCell& cell) const {
  double count = -1.0;
  RegularCell::entry_t_ptr p;
  for (const auto& e : cell) {
      if (e.second->getCount() > count){
          p = e.second;
          count = p->getCount();
      }
  }
  p->setSelectionRank(0.0);
  return p;
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
