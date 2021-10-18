/*
 * INeighborhoodTable.h
 *
 *  Created on: Oct 5, 2021
 *      Author: vm-sts
 */

#pragma once

#include "inet/common/geometry/common/Coord.h"
#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "crownet/common/iterator/FilterIterator.h"

namespace crownet {


using NeighborhoodTable_t = std::map<int, BeaconReceptionInfo*>;
using NeighborhoodTablePred_t = std::function<bool(const NeighborhoodTable_t::value_type&)>;
using NeighborhoodTableValue_t = NeighborhoodTable_t::value_type;
using NeighborhoodTableIter_t = FilterIterator<NeighborhoodTable_t, NeighborhoodTable_t::iterator, NeighborhoodTableValue_t, NeighborhoodTablePred_t>;

class IBaseNeighborhoodTable{
public:
    virtual ~IBaseNeighborhoodTable() = default;


    virtual NeighborhoodTableIter_t iter() = 0;
    virtual NeighborhoodTableIter_t iter(NeighborhoodTablePred_t predicate) = 0;

    // Iterator and predicate definitions
    static NeighborhoodTablePred_t all_pred(){
        NeighborhoodTablePred_t f =  [](const NeighborhoodTableValue_t& val ) -> bool { return true;};
        return f;
    }

    static NeighborhoodTablePred_t inRadius_pred(const inet::Coord& pos, const double dist ){
        NeighborhoodTablePred_t f = [pos, dist](const NeighborhoodTableValue_t& val) -> bool {
            return pos.distance(val.second->getPos()) < dist;
        };
        return f;
    }

    static NeighborhoodTableIter_t all(NeighborhoodTable_t* data){
        return NeighborhoodTableIter_t(data, all_pred());
    }

    static NeighborhoodTableIter_t inRadius(NeighborhoodTable_t* data, const inet::Coord& pos, const double dist){
        return NeighborhoodTableIter_t(data, inRadius_pred(pos, dist));
    }
};


class INeighborhoodTable : public IBaseNeighborhoodTable{
public:
    virtual ~INeighborhoodTable() = default;

    virtual BeaconReceptionInfo* getOrCreateEntry(const int sourceId) = 0;
    // update table and remove old entries. Update local entry
    virtual void checkTimeToLive() = 0;
    virtual void setOwnerId(int ownerId) = 0;
    virtual const int getOwnerId() const = 0;

};


}


