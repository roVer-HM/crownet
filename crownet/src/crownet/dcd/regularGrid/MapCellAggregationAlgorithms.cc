/*
 * Cell.cc
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include <math.h>
#include <algorithm>
#include <vector>
#include <omnetpp.h>

namespace crownet {


RegularCell::entry_t_ptr YmfVisitor::applyTo(const RegularCell& cell)  {
  RegularCell::entry_t_ptr ret = nullptr;
  // cellIter -> valid entries only!
  for (const auto& e : this->cellIter(cell)) {
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
  return update_selection(ret, YmfVisitor::COPY_TRUE);
}


YmfPlusDistVisitor::sum_data YmfPlusDistVisitor::getSums(const RegularCell& cell) const  {
    double age_sum = 0.0;
    double age_min = std::numeric_limits<double>::max();
    double distance_sum = 0.0;
    double dist_min = std::numeric_limits<double>::max();
    double now = this->time.dbl(); // current time the visitor is called.

    double age = .0;
    double dist = .0;
    int count = 0;
    // cellIter -> valid entries only!
    for (const auto& e : this->cellIter(cell)) {
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

RegularCell::entry_t_ptr YmfPlusDistVisitor::applyTo(const RegularCell& cell)  {

    sum_data d = getSums(cell);
    double now = this->time.dbl(); // current time the visitor is called.

    RegularCell::entry_t_ptr ret = nullptr;
    double ret_ymfPlusDist = 0.0;
    double ymfPlusDist = 0.0;
    // cellIter -> valid entries only!
    for (const auto& e : this->cellIter(cell)) {
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
    return  update_selection(ret, YmfPlusDistVisitor::COPY_TRUE);
}


RegularCell::entry_t_ptr YmfPlusDistStepVisitor::applyTo(const RegularCell& cell)  {

    sum_data d = getSums(cell);
    double now = this->time.dbl(); // current time the visitor is called.

    RegularCell::entry_t_ptr ret = nullptr;
    double ret_ymfPlusDist = 0.0;
    double ymfPlusDist = 0.0;
    // cellIter -> valid entries only!
    for (const auto& e : this->cellIter(cell)) {
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
    return  update_selection(ret, YmfPlusDistStepVisitor::COPY_TRUE);
}

const double YmfPlusDistStepVisitor::getDistValue(const double dist) const {
        return (dist <= stepDist) ? stepDist : dist;
}

YmfPlusDistVisitor::sum_data YmfPlusDistStepVisitor::getSums(const RegularCell& cell)  const {
    double age_sum = 0.0;
    double age_min = std::numeric_limits<double>::max();
    double distance_sum = 0.0;
    double dist_min = std::numeric_limits<double>::max();
    double now = this->time.dbl(); // current time the visitor is called.

    double age = .0;
    double dist = .0;
    int count = 0;
    // cellIter -> valid entries only!
    for (const auto& e : this->cellIter(cell)) {
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

RegularCell::entry_t_ptr LocalSelector::applyTo(const RegularCell& cell)  {
    // to check local exists.... raise error.....
    auto val = cell.getLocal();
    val->setSelectionRank(0.0);
    return update_selection(val, LocalSelector::COPY_TRUE);
}


RegularCell::entry_t_ptr MeanVisitor::applyTo(const RegularCell& cell)  {
  double sum = 0;
  double num = 0;
  double time = 0;
  // cellIter -> valid entries only!
  for (const auto& e : this->cellIter(cell)) {
      ++num;
      sum += e.second->getCount();
      time  += e.second->getMeasureTime().dbl(); // todo
  }
  auto entry = cell.createEntry(sum/num); // new entry created here!
  entry->setMeasureTime(time/num);
  entry->setReceivedTime(time/num);
  entry->setSelectionRank(0.0);
  return update_selection(entry, MeanVisitor::COPY_FALSE);
}

RegularCell::entry_t_ptr MedianVisitor::applyTo(const RegularCell& cell)  {
  std::vector<double> count;
  std::vector<double> time;
  // cellIter -> valid entries only!
  for (const auto& e : this->cellIter(cell)) {
      count.push_back(e.second->getCount());
      time.push_back(e.second->getMeasureTime().dbl());
  }
  auto m  = count.begin() + count.size()/2;
  int left = (int)(count.size()-1)/2; // lower-right
  int right = (int)count.size()/2;    // upper-left
  std::nth_element(count.begin(), m, count.end()); // only sort half of the data
  std::nth_element(time.begin(), m, time.end()); // only sort half of the data


  auto entry = cell.createEntry((double)(count[left] + count[right])/2);  // new entry created here!
  entry->setMeasureTime((double)(count[left] + count[right])/2);
  entry->setReceivedTime((double)(time[left] + time[right])/2);
  entry->setSelectionRank(0.0);
  return update_selection(entry, MedianVisitor::COPY_FALSE);
}

RegularCell::entry_t_ptr InvSourceDistVisitor::applyTo(const RegularCell& cell)  {
  double itemSum = 0;
  double weightSum = 0;
  // cellIter -> valid entries only!

  for (const auto& e :  this->cellIter(cell)) {
      EntryDist dist = e.second->getEntryDist();
      double w = 1.0 / std::max(1.0, dist.sourceEntry);
      weightSum += w;
      itemSum += w * e.second->getCount();
  }
  auto entry = cell.createEntry(itemSum/weightSum);  // new entry created here!
  entry->touch(this->time);
  entry->setSelectionRank(0.0);
  return update_selection(entry, InvSourceDistVisitor::COPY_FALSE);
}

RegularCell::entry_t_ptr MaxCountVisitor::applyTo(const RegularCell& cell)  {
  double count = -1.0;
  RegularCell::entry_t_ptr p;
  for (const auto& e : this->cellIter(cell)) {
      if (e.second->getCount() > count){
          p = e.second;
          count = p->getCount();
      }
  }
  p->setSelectionRank(0.0);
  return update_selection(p, MaxCountVisitor::COPY_TRUE);
}

RegularCell::entry_t_ptr AlgSmall::applyTo(const RegularCell& cell)  {
  RegularCell::value_type ret;
  for (const auto& e : this->cellIter(cell)) {
    if (!ret.second) ret.second = e.second;
    if (e.second->getCount() < ret.second->getCount()) {
      ret = std::make_pair(e.first, e.second);
    }
  }
  return update_selection(ret.second, AlgSmall::COPY_TRUE);
}

}  // namespace crownet
