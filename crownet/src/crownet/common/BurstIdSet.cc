/*
 * FixeSizeSet.cc
 *
 *  Created on: Feb 15, 2023
 *      Author: vm-sts
 */

#include "BurstIdSet.h"

namespace crownet {



bool BurstIdSet::add(const simtime_t id){
    if (contains(id)){
        return false;
    } else {
        if (burst_ids.size() < setSize){
            return burst_ids.emplace(id).second;
        } else {
            auto iter = burst_ids.begin();
            //erase oldest value
            burst_ids.erase(iter);
            return burst_ids.emplace(id).second;
        }
    }
}

bool BurstIdSet::contains(const simtime_t id) const {
    return burst_ids.find(id) != burst_ids.end();
}

const simtime_t BurstIdSet::getSmallestValue() const{
    if (burst_ids.size() > 1){
        return *burst_ids.begin();
    } else {
        return -1.0;
    }
}
const simtime_t BurstIdSet::getLargestValue() const{
    if (burst_ids.size() > 1){
        return *burst_ids.rbegin();
    } else {
        return -1.0;
    }
}


} /* namespace crownet */
