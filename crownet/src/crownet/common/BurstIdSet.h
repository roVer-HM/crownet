/*
 * BurstIdSet.h
 *
 *  Created on: Feb 15, 2023
 *      Author: vm-sts
 */

#ifndef CROWNET_COMMON_BURSTIDSET_H_
#define CROWNET_COMMON_BURSTIDSET_H_

#include "crownet/crownet.h"
#include <set>
#include <functional>

namespace crownet {

class BurstIdSet {
public:
    BurstIdSet(int setSize=30)
        : setSize(setSize){
    };
    virtual ~BurstIdSet() = default;

    bool add(const simtime_t id);
    bool contains(const simtime_t id) const;
    const simtime_t getSmallestValue() const;
    const simtime_t getLargestValue() const;
    const int size() const {return burst_ids.size();}

private:
    int setSize;
    std::set<simtime_t, std::less<simtime_t> > burst_ids;
};

} /* namespace crownet */

#endif /* CROWNET_COMMON_BURSTIDSET_H_ */
