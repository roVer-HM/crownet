/*
 * INeighborhoodTable.h
 *
 *  Created on: Oct 5, 2021
 *      Author: vm-sts
 */

#pragma once
#include "crownet/crownet.h"
#include "inet/common/geometry/common/Coord.h"
#include "crownet/applications/beacon/BeaconReceptionInfo.h"
#include "crownet/common/converter/OsgCoordConverter.h"
#include "crownet/common/iterator/FilterIterator.h"
#include <list>

namespace crownet {



using NeighborhoodTable_t = std::map<int, BeaconReceptionInfo*>;
using NeighborhoodTablePred_t = std::function<bool(const NeighborhoodTable_t::value_type&)>;
using NeighborhoodTableValue_t = NeighborhoodTable_t::value_type;
using NeighborhoodTableIter_t = FilterIterator<NeighborhoodTable_t, NeighborhoodTable_t::iterator, NeighborhoodTableValue_t, NeighborhoodTablePred_t>;
class NeighborhoodEntryListner;

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
            return pos.distance(val.second->getPositionCurrent()) < dist;
        };
        return f;
    }

    static NeighborhoodTablePred_t currentCell_pred(const inet::Coord& pos, const RegularGridInfo& grid){
        NeighborhoodTablePred_t f = [pos, grid](const NeighborhoodTableValue_t& tblEntry) -> bool {

            return grid.posInCenteredCell(tblEntry.second->getPositionCurrent(), pos);
        };
        return f;
    }

    static NeighborhoodTableIter_t all(NeighborhoodTable_t* data){
        return NeighborhoodTableIter_t(data, all_pred());
    }

    static NeighborhoodTableIter_t inRadius(NeighborhoodTable_t* data, const inet::Coord& pos, const double dist){
        return NeighborhoodTableIter_t(data, inRadius_pred(pos, dist));
    }

    const simtime_t getLastUpdatedAt() const { return lastUpdated; }
protected:
    void setLastUpdatedAt(const simtime_t t) { lastUpdated = t;}

private:
    simtime_t lastUpdated = -1.0;

};


class INeighborhoodTable : public IBaseNeighborhoodTable{
public:
    virtual ~INeighborhoodTable() = default;

    virtual BeaconReceptionInfo* getOrCreateEntry(const int sourceId) = 0;
    virtual bool processInfo(BeaconReceptionInfo *packet) = 0;
    // update table and remove old entries. Update local entry
    virtual void checkAllTimeToLive() = 0;
    virtual bool ttlReached(BeaconReceptionInfo*)=0;
    virtual void setOwnerId(int ownerId) = 0;
    virtual const int getOwnerId() const = 0;
    virtual const int getSize() = 0;



    void registerEntryListner(NeighborhoodEntryListner* listener);
    void registerFirst(NeighborhoodEntryListner* listener);
    void removeEntryListener(NeighborhoodEntryListner* listener);
    void emitRemoved(BeaconReceptionInfo* info);
    void emitDropped(BeaconReceptionInfo* info);
    void emitLeaveCell(BeaconReceptionInfo* info);
    void emitEnterCell(BeaconReceptionInfo* info);
    void emitStayInCell(BeaconReceptionInfo* info);


protected:
    std::list<NeighborhoodEntryListner*> listeners;
};


class NeighborhoodEntryListner {
public:
    virtual ~NeighborhoodEntryListner() = default;
    virtual void neighborhoodEntryRemoved(INeighborhoodTable* table, BeaconReceptionInfo* info)=0;
    virtual void neighborhoodEntryLeaveCell(INeighborhoodTable* table, BeaconReceptionInfo* info)=0;
    virtual void neighborhoodEntryEnterCell(INeighborhoodTable* table, BeaconReceptionInfo* info)=0;
    virtual void neighborhoodEntryStayInCell(INeighborhoodTable* table, BeaconReceptionInfo* info)=0;
    virtual void neighborhoodEntryDropped(INeighborhoodTable* table, BeaconReceptionInfo* info){/*do nothing*/};
};


}


