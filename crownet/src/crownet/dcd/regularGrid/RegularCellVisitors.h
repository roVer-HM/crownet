/*
 * RegularCellVisitor.h
 *
 *  Created on: Nov 23, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/CellVisitors.h"
#include "crownet/dcd/regularGrid/RegularCell.h"

namespace crownet {
// test only makes no sense to increment all IEntries
class IncrementerVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  IncrementerVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override {
    for (auto e : cell) {
      e.second->incrementCount(time);
    }
  }
};

/**
 * Reset all IEntries within a RegularCell
 */
class ResetVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ResetVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override {
    for (auto e : cell) {
      e.second->reset(time);
    }
  }
};

/**
 * Clear all IEntries within a RegularCell
 */
class ClearVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ClearVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override {
    for (auto e : cell) {
      e.second->clear(time);
    }
  }
};

class ClearSelection : public VoidCellVisitor<RegularCell> {
 public:
  ClearSelection() {}
  virtual void applyTo(RegularCell& cell) override {
    for (auto e : cell) {
      e.second->setSelectedIn("");
    }
  }
};

class ClearLocalVisitor : public TimestampedVoidCellVisitor<RegularCell> {
 public:
  ClearLocalVisitor(RegularCell::time_t time)
      : TimestampedVoidCellVisitor<RegularCell>(time) {}
  virtual void applyTo(RegularCell& cell) override {
    if (cell.hasLocal()) {
      cell.getLocal()->clear(time);
    }
  }
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
      : TimestampedGetEntryVisitor<RegularCell>(t) {}
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
  virtual std::string getName() const override { return "ymf"; }
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
    virtual std::string getName() const override { return "mean"; }
};

class MedianVisitor : public TimestampedGetEntryVisitor<RegularCell> {
public:
    MedianVisitor(RegularCell::time_t t = 0.0) :
        TimestampedGetEntryVisitor<RegularCell>(t) {}
    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) const override;
    virtual std::string getName() const override { return "mean"; }
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
    virtual std::string getName() const override { return "invSourceDist"; }
};



class MaxCountVisitor : public TimestampedGetEntryVisitor<RegularCell> {
 public:
    MaxCountVisitor(RegularCell::time_t t = 0.0) :
        TimestampedGetEntryVisitor<RegularCell>(t) {}

  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
  virtual std::string getName() const override { return "maxCount"; }

};

class AlgSmall : public GetEntryVisitor<RegularCell> {
 public:
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) const override;
};
}  // namespace crownet
