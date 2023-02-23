/*
 * NeighborhoodTable.h
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#pragma once

#include <omnetpp/cobject.h>
#include "inet/common/InitStages.h"
#include "crownet/neighbourhood/contract/INeighborhoodTable.h"
#include "crownet/common/converter/OsgCoordConverter.h"
#include "crownet/dcd/identifier/CellKeyProvider.h"
#include "crownet/common/MobilityProviderMixin.h"

using namespace omnetpp;
using namespace inet;

namespace crownet {

class NeighborhoodTableSizeFilter;

class NeighborhoodTable : public MobilityProviderMixin<cSimpleModule>
                          , public INeighborhoodTable
                          , public INeighborhoodTablePacketProcessor
{
public:

    friend NeighborhoodTableSizeFilter;

    virtual ~NeighborhoodTable();

    // cSimpleModule
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;

    virtual bool ttlReached(BeaconReceptionInfo*) override;
    virtual void checkAllTimeToLive() override;
    virtual const int getSize() override;

    virtual bool processInfo(BeaconReceptionInfo *packet) override;
    virtual void saveInfo(BeaconReceptionInfo* info) override;
    virtual const BeaconReceptionInfo* find(int sourceId) const override;

    //getter
    const simtime_t& getMaxAge() const { return maxAge; }
    const NeighborhoodTable_t& getTable() const { return _table; }
    const cMessage* getTitleMessage() const { return ttl_msg;}
    virtual const int getOwnerId() const override { return ownerId;}


    //setter
    void setMaxAge(const simtime_t& _maxAge) { maxAge = _maxAge; }
    void setTable(const std::map<int, BeaconReceptionInfo*>& _nTable){ _table = _nTable;}
    void setTitleMessage(cMessage *msg){ttl_msg = msg;}
    void setOwnerId(int ownerId) override {this->ownerId = ownerId;}


    // iterator default to all elements in map
    virtual NeighborhoodTableIter_t iter() override {
        return IBaseNeighborhoodTable::all(&_table);
    }

    virtual NeighborhoodTableIter_t iter(NeighborhoodTablePred_t predicate) override {
        return NeighborhoodTableIter_t(&_table, predicate);
    }

protected:
    virtual void removeInfo(BeaconReceptionInfo* info);


protected:
    int ownerId;
    int tableSize;
    NeighborhoodTable_t _table;
    simtime_t maxAge;
    cMessage *ttl_msg = nullptr;
    simtime_t lastCheck;
    std::shared_ptr<GridCellIDKeyProvider> cellKeyProvider;
};


class NeighborhoodTableSizeFilter : public cObjectResultFilter {
public:
 virtual void receiveSignal(cResultFilter *prev, simtime_t_cref t,
                            cObject *object, cObject *details) override;
 using cObjectResultFilter::receiveSignal;
private:
 simtime_t last_receivedSignal = -1.0;
 int lastSize = -1;

};

} /* namespace crownet */

