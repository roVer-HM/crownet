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
using BaseCellVisitor = MapMixin<TimestampMixin<VoidCellVisitor<RegularCell>, typename RegularCell::time_t>, RegularDcdMap>;

class IncrementerVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  IncrementerVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override;
};

/**
 * Reset all IEntries within a RegularCell
 */
class ResetVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ResetVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
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
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override;
};


class TTLCellAgeHandler : public IdenpotanceTimestampedVoidCellVisitor<RegularCell> {
public:
    TTLCellAgeHandler(RegularDcdMapPtr dcdMap, RegularCell::time_t ttl, RegularCell::time_t t = 0.0)
     : IdenpotanceTimestampedVoidCellVisitor<RegularCell>(t), dcdMap(dcdMap), ttl(ttl){}
 virtual void applyTo(RegularCell& cell) override;
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
  ClearSelection() {}
  virtual void applyTo(RegularCell& cell) override;
};


/**
 *  Simple clear operation on all cells containing local measurements.
 *  1. Set count of existing valid cells to zero.
 *  2. Update time stamp to set time of the visitor (TimestampMixin)
 */
class ClearLocalVisitor : public BaseCellVisitor {
 public:
  ClearLocalVisitor(){}
  ClearLocalVisitor(const ClearLocalVisitor& other): BaseCellVisitor(other) {}
//  ClearLocalVisitor(RegularCell::time_t time) : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual cObject *dup() const override { return new ClearLocalVisitor(*this); }
  virtual void applyTo(RegularCell& cell) override;
};


/**
 *  Clear operation on all cells containing local measurements.
 *  When updating the (local) density map with data from the neighborhood table (NT)
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
class TtlClearLocalVisitor : public ClearLocalVisitor{
public:
    TtlClearLocalVisitor(): zeroTtl(0.0), keepZeroDistance(-1.0) {};
    TtlClearLocalVisitor(const TtlClearLocalVisitor& other): ClearLocalVisitor(other) {
        this->zeroTtl = other.getZeroTtl();
        this->keepZeroDistance = other.getKeepZeroDistance();
    }
    virtual cObject *dup() const override { return new TtlClearLocalVisitor(*this); }
    virtual void applyTo(RegularCell& cell) override;

    const simtime_t getZeroTtl() const { return zeroTtl;}
    void setZeroTtl(const simtime_t& zeroTtl) {this->zeroTtl = zeroTtl;}

    const double getKeepZeroDistance() const { return keepZeroDistance;}
    void setKeepZeroDistance(const double& keepZeroDistance) {this->keepZeroDistance = keepZeroDistance;}
private:
    simtime_t zeroTtl; // time to live for entries with count = 0;
    double keepZeroDistance; //
};


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
 * Return Youngest Measurement from a RegularCell
 *
 * Only look at valid items.
 */
class YmfVisitor : public TimestampedGetEntryVisitor<RegularCell> {
 public:
    YmfVisitor(RegularCell::time_t t = 0.0)
      : TimestampedGetEntryVisitor<RegularCell>(t){}
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
  virtual std::string getVisitorName() const override { return "ymf"; }

};


class YmfPlusDistVisitor : public TimestampedGetEntryVisitor<RegularCell> {
protected:
    struct sum_data {
        double age_sum;
        double age_min;
        double dist_sum;
        double dist_min;
        int count;
    };
public:
    YmfPlusDistVisitor(double alpha = 0.5, RegularCell::time_t t = 0.0)
        : TimestampedGetEntryVisitor<RegularCell>(t), alpha(alpha) {}
    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) const override;
    virtual sum_data getSums(const RegularCell& cell) const;
    virtual std::string getVisitorName() const override { return "ymfPlusDist"; }

protected:
    double alpha;
};

class YmfPlusDistStepVisitor : public YmfPlusDistVisitor {
public:
    YmfPlusDistStepVisitor(double alpha, RegularCell::time_t t, double stepDist)
        : YmfPlusDistVisitor(alpha, t), stepDist(stepDist) {}
    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) const override;
    virtual sum_data getSums(const RegularCell& cell) const override;
    virtual const double getDistValue(const double dist) const;
    virtual std::string getVisitorName() const override { return "ymfPlusDistStep"; }

protected:
    double stepDist;
};

class LocalSelector : public TimestampedGetEntryVisitor<RegularCell> {
 public:
    LocalSelector(RegularCell::time_t t = 0.0)
      : TimestampedGetEntryVisitor<RegularCell>(t) {}
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
  virtual std::string getVisitorName() const override { return "local"; }
};



// class LocalVisitor : public GetCellVisitor<RegularCell> {
// public:
//  virtual RegularCell::value_type applyTo(
//      const RegularCell& cell) const override;
//};

/**
 * Return mean measurement with all cells weighted equally
 */
class MeanVisitor : public TimestampedGetEntryVisitor<RegularCell> {
public:
    MeanVisitor(RegularCell::time_t t = 0.0) :
        TimestampedGetEntryVisitor<RegularCell>(t) {}
    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) const override;
    virtual std::string getVisitorName() const override { return "mean"; }
};

class MedianVisitor : public TimestampedGetEntryVisitor<RegularCell> {
public:
    MedianVisitor(RegularCell::time_t t = 0.0) :
        TimestampedGetEntryVisitor<RegularCell>(t) {}
    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) const override;
    virtual std::string getVisitorName() const override { return "mean"; }
};


/**
 * Return mean measurement with weighted based on the distance between the
 * cell origin and the Entry owner
 */
class InvSourceDistVisitor : public TimestampedGetEntryVisitor<RegularCell> {
public:
    InvSourceDistVisitor(RegularCell::time_t t = 0.0) :
        TimestampedGetEntryVisitor<RegularCell>(t) {}
    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) const override;
    virtual std::string getVisitorName() const override { return "invSourceDist"; }
};



class MaxCountVisitor : public TimestampedGetEntryVisitor<RegularCell> {
 public:
    MaxCountVisitor(RegularCell::time_t t = 0.0) :
        TimestampedGetEntryVisitor<RegularCell>(t) {}

  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
  virtual std::string getVisitorName() const override { return "maxCount"; }

};

class AlgSmall : public GetEntryVisitor<RegularCell> {
 public:
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
};
}  // namespace crownet
