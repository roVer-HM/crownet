/*
 * RegularCellVisitor.h
 *
 *  Created on: Nov 23, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/CellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

namespace crownet {

/**
 * A VoidCellVisitor with access to current time and the underlining density map as a shared pointer.
 */
using BaseCellVisitor = MapMixin<TimestampMixin<VoidCellVisitor<RegularCell>>, RegularDcdMap>;

class IncrementerVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  IncrementerVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) { }
  virtual void applyTo(RegularCell& cell) override;
};

/**
 * Reset all IEntries within a RegularCell
 */
class ResetVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ResetVisitor(RegularCell::time_t time) : TimestampedVoidCellVisitor<RegularCell>(time) { }
  virtual void applyTo(RegularCell& cell) override;
};

/**
 * Reset local IEntries in a RegularCell
 */
class ResetLocalVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
    ResetLocalVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override;
};

/**
 * Clear all IEntries within a RegularCell
 */
class ClearVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ClearVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) { }
  virtual void applyTo(RegularCell& cell) override;
};


class TTLCellAgeHandler : public IdenpotanceTimestampedVoidCellVisitor<RegularCell> {
public:
    TTLCellAgeHandler(RegularDcdMapPtr dcdMap, RegularCell::time_t ttl, RegularCell::time_t t = 0.0)
     : IdenpotanceTimestampedVoidCellVisitor<RegularCell>(t), dcdMap(dcdMap), ttl(ttl){
        // apply to all cells.
        this->entryPredicate = CellDataIterator<RegularCell>::getAllDataIter_pred();
    }
 virtual void applyIfChanged(RegularCell& cell) override;
 virtual std::string getVisitorName() const override { return "silentCellAgeHandler"; }

protected:
 RegularDcdMapPtr dcdMap;
 RegularCell::time_t ttl;
};

/**
 * Clear 'selectedIn' property of all entries in a RegularCell
 */
class ClearSelection : public VoidCellVisitor<RegularCell> {
 public:
  ClearSelection(): VoidCellVisitor<RegularCell>() {}
  ClearSelection(typename CellDataIterator<RegularCell>::pred_t& pred): VoidCellVisitor<RegularCell>(pred) {}
  virtual void applyTo(RegularCell& cell) override;
};


/**
 *  Simple clear operation on all cells containing local measurements.
 *  1. Set count of existing valid cells to zero.
 *  2. Update time stamp to set time of the visitor (TimestampMixin)
 */
class ClearLocalVisitor : public BaseCellVisitor {
 public:
//  ClearLocalVisitor(): BaseCellVisitor(){}
//  ClearLocalVisitor(const ClearLocalVisitor& other): BaseCellVisitor(other) {}
//  ClearLocalVisitor(RegularCell::time_t time) : TimestampedVoidCellVisitor<RegularCell>(time) {}
//  virtual cObject *dup() const override { return new ClearLocalVisitor(*this); }
  virtual void applyTo(RegularCell& cell) override;
};


/**
 *  Clear operation on all cells containing local measurements.
 *  When updating the (local) density map with data from the neighborhood table (NT),
 *  one has to take into account the moving of nodes between cells. If node 'A' moves
 *  from one cell C_1 to cell C_2 the overall count of C_1 must be decrement by one and
 *  the C_2 must be incremented. However because the NT only contains the current location
 *  of node 'A' it is not possible to decrement the old cell. Therefore during the
 *  update of the local density map _all_ cells previously containing at least one count
 *  are set to zero with the current time set. Afterwards, a loop over all entries in the NT
 *  will again increment all known locations. If a cell change as described above happened.
 *  Cell C_2 will be incremented normal. C_1 however has no corresponding entry in the
 *  NT and thus does not get incremented. Because of the active rest to zero and the update
 *  of the time stamp the density map contains an active zero value which will be propagated.
 *
 *  Assume during the next update cycle of the local density map no new entry in the NT
 *  points to cell C_1. Thus the value of zero is still the 'newest' value known for this
 *  cell. Therefore this value is kept without changing it's time stamp to allow the zero
 *  value to 'age' normally like any other value.  After a given zeroTtl a zero value will
 *  be removed completely (set to invalid). If 'keepZeroDistance' is > 0.0  a zero value
 *  will *NOT* be deleted if the cells position is within keepZeroDistance of the current
 *  nodes location.
 *
 */
// TODO: deprecated?
class TtlClearLocalVisitor : public ClearLocalVisitor{
public:
    TtlClearLocalVisitor(): ClearLocalVisitor(), zeroTtl(0.0), keepZeroDistance(-1.0) {};
//    TtlClearLocalVisitor(const TtlClearLocalVisitor& other): ClearLocalVisitor(other) {
//        this->zeroTtl = other.getZeroTtl();
//        this->keepZeroDistance = other.getKeepZeroDistance();
//    }
//    virtual cObject *dup() const override { return new TtlClearLocalVisitor(*this); }
    virtual void applyTo(RegularCell& cell) override;

    const simtime_t getZeroTtl() const { return zeroTtl;}
    void setZeroTtl(const simtime_t& zeroTtl) {this->zeroTtl = zeroTtl;}

    const double getKeepZeroDistance() const { return keepZeroDistance;}
    void setKeepZeroDistance(const double& keepZeroDistance) {this->keepZeroDistance = keepZeroDistance;}
private:
    simtime_t zeroTtl; // time to live for entries with count = 0;
    double keepZeroDistance; //
};

// TODO: deprecated?
class ClearCellIdVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ClearCellIdVisitor(RegularCell::node_key_t id, RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time), id(id) {}
  virtual void applyTo(RegularCell& cell) override {
    if (cell.hasData(id)) {
      cell.get(id)->clear(time);
    }
  }

 private:
  RegularCell::node_key_t id;
};



/**
 *  Apply resource sharing domain id (RSD-id) to the computed value of each cell. Only cell
 *  which are part of the local map (LM) are by definition of the LM in the same RSD as the
 *  map owner. The LM is constructed solely by 1-hop direct communication, which entails that
 *  the sender and receiver (i.e. map owner) use the same RSD.
 */
class ApplyRessourceSharingDomainIdVisitor : public IdenpotanceTimestampedVoidCellVisitor<RegularCell> {
 public:
    ApplyRessourceSharingDomainIdVisitor(RegularCell::time_t time = 0.0)
        : IdenpotanceTimestampedVoidCellVisitor<RegularCell>(time){}
    ApplyRessourceSharingDomainIdVisitor(RegularCell::time_t time, typename CellDataIterator<RegularCell>::pred_t& pred)
        : IdenpotanceTimestampedVoidCellVisitor(time, pred) {}

 public:
    virtual void applyIfChanged(RegularCell& cell) override;


};

class RsdNeighborhoodCountVisitor : public IdenpotanceTimestampedVoidCellVisitor<RegularCell>{
  public:
    RsdNeighborhoodCountVisitor(RegularCell::time_t time = 0.0, int rsdid = -1)
        : IdenpotanceTimestampedVoidCellVisitor<RegularCell>(time, CellDataIterator<RegularCell>::getValidDataIter_pred()), rsdid(rsdid), count(0){}

    virtual void applyIfChanged(RegularCell& cell) override;
    virtual void reset(RegularCell::time_t time, int rsdid);
    virtual int getCount() const {return count;}

  protected:
    int rsdid;
    int count;

};

class FullNeighborhoodCountVisitor: public RsdNeighborhoodCountVisitor {
public:
    FullNeighborhoodCountVisitor(RegularCell::time_t time = 0.0, int rsdid = -1)
      : RsdNeighborhoodCountVisitor(time, rsdid){}

    virtual void applyTo(RegularCell& cell) override;
};



}  // namespace crownet
