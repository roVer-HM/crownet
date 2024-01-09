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
  for (auto e : this->cellIter(cell)) {
    e.second->incrementCount(time);
  }
}


void ResetVisitor::applyTo(RegularCell& cell) {
  for (auto e : this->cellIter(cell)) {
    e.second->reset(time);
  }
}

void ResetLocalVisitor::applyTo(RegularCell& cell) {
    if (cell.hasLocal()) {
      cell.getLocal()->reset(time);
    }
  }


void ClearVisitor::applyTo(RegularCell& cell) {
  for (auto e : this->cellIter(cell)) {
    e.second->clear(time);
  }
}

void TTLCellAgeHandler::applyIfChanged(RegularCell& cell) {
    for (auto e : this->cellIter(cell)) {
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

void ClearSelection::applyTo(RegularCell& cell) {
  for (auto e : this->cellIter(cell)) {
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


void ApplyRessourceSharingDomainIdVisitor::reset(RegularCell::time_t time, int rsdid){
    this->rsdid = rsdid;
    this->time = time;
    this->count = 0;
}

void ApplyRessourceSharingDomainIdVisitor::applyIfChanged(RegularCell& cell) {
    // if valid local measurement exist apply that RSD-ID to the calculated value of
    // this cell as this cell is inside of the RSD domain.
    if(cell.hasValid()){
        auto calcualtedValue = cell.val();
        if (calcualtedValue && cell.hasLocal()){
            // cell is in reach of owner, set owners RSD
            int localRSD = cell.getLocal()->getResourceSharingDomainId();
            calcualtedValue->setResourceSharingDomainId(localRSD);
        } else if (calcualtedValue){
            // The map owner has no own measurement for this cell. If any FAM measurement has the same
            // RSD as the owner than the cell belongs to the same RSD. Otherwise
            for(const auto& entry : this->cellIter(cell)){
                if (entry.second->getResourceSharingDomainId() == rsdid){
                    calcualtedValue->setResourceSharingDomainId( rsdid);
                    break;
                }
            }
        } else {
            throw cRuntimeError("No computed value or cell '%s' of DPM map of node '%s' present when applying resource sharing domain identifier",
                    cell.getCellId().str().c_str(), cell.getOwnerId().str().c_str());
        }
    }

    if (rsdid < 0){
        count = 1; // ego count only.
        return;
    }

    if (cell.val() && cell.val()->getResourceSharingDomainId() == rsdid){
        count = count + cell.val()->getCount();
    }

}

void RsdNeighborhoodCountVisitor::applyIfChanged(RegularCell& cell){

    if (rsdid < 0){
        count = 1; // ego count only.
        return;
    }

    if (cell.val() && cell.val()->getResourceSharingDomainId() == rsdid){
        count = count + cell.val()->getCount();
    }
}


void FullNeighborhoodCountVisitor::applyTo(RegularCell& cell){
    // ignore rsd an count all.
    if (cell.val()){
        count = count + cell.val()->getCount();
    }
}



void RsdNeighborhoodCountVisitor::reset(RegularCell::time_t time, int rsdid){
    this->count = 0;
    this->rsdid = rsdid;
    this->time = time;
}

}  // namespace crownet
