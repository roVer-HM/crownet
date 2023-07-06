
#pragma once

#include "crownet/dcd/generic/CellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

namespace crownet {
//CellAggregationAlgorihm<RegularCell>


/**
 * Return Youngest Measurement from a RegularCell
 *
 * Only look at valid items.
 */
class YmfVisitor : public CellAggregationAlgorihm<RegularCell> {
 public:
    YmfVisitor(RegularCell::time_t t = 0.0)
      : CellAggregationAlgorihm<RegularCell>(t, CellDataIterator<RegularCell>::getValidDataIter_pred()){}
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) override;
  virtual std::string getVisitorName() const override { return "ymf"; }

};


class YmfPlusDistVisitor : public CellAggregationAlgorihm<RegularCell> {
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
        : CellAggregationAlgorihm<RegularCell>(t, CellDataIterator<RegularCell>::getValidDataIter_pred()), alpha(alpha){}

    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) override;
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
        const RegularCell& cell)  override;
    virtual sum_data getSums(const RegularCell& cell) const override;
    virtual const double getDistValue(const double dist) const;
    virtual std::string getVisitorName() const override { return "ymfPlusDistStep"; }

protected:
    double stepDist;
};

class LocalSelector : public CellAggregationAlgorihm<RegularCell> {
 public:
    LocalSelector(RegularCell::time_t t = 0.0)
      : CellAggregationAlgorihm<RegularCell>(t) {}
  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) override;
  virtual std::string getVisitorName() const override { return "local"; }
};

/**
 * Return mean measurement with all cells weighted equally
 */
class MeanVisitor : public CellAggregationAlgorihm<RegularCell> {
public:
    MeanVisitor(RegularCell::time_t t = 0.0) :
        CellAggregationAlgorihm<RegularCell>(t, CellDataIterator<RegularCell>::getValidDataIter_pred()) {}

    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) override;
    virtual std::string getVisitorName() const override { return "mean"; }
};

class MedianVisitor : public CellAggregationAlgorihm<RegularCell> {
public:
    MedianVisitor(RegularCell::time_t t = 0.0) :
        CellAggregationAlgorihm<RegularCell>(t, CellDataIterator<RegularCell>::getValidDataIter_pred()) {}

    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) override;
    virtual std::string getVisitorName() const override { return "mean"; }
};


/**
 * Return mean measurement with weighted based on the distance between the
 * cell origin and the Entry owner
 */
class InvSourceDistVisitor : public CellAggregationAlgorihm<RegularCell> {
public:
    InvSourceDistVisitor(RegularCell::time_t t = 0.0) :
        CellAggregationAlgorihm<RegularCell>(t, CellDataIterator<RegularCell>::getValidDataIter_pred()) {}

    virtual RegularCell::entry_t_ptr applyTo(
        const RegularCell& cell) override;
    virtual std::string getVisitorName() const override { return "invSourceDist"; }
};



class MaxCountVisitor : public CellAggregationAlgorihm<RegularCell> {
 public:
    MaxCountVisitor(RegularCell::time_t t = 0.0) : CellAggregationAlgorihm<RegularCell>(t) {}
    MaxCountVisitor(RegularCell::time_t t, typename CellDataIterator<RegularCell>::pred_t& pred) : CellAggregationAlgorihm<RegularCell>(t, pred) {}

  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) override;
  virtual std::string getVisitorName() const override { return "maxCount"; }

};

class AlgSmall : public CellAggregationAlgorihm<RegularCell> {
 public:
    AlgSmall() : CellAggregationAlgorihm<RegularCell>() {}
    AlgSmall(typename CellDataIterator<RegularCell>::pred_t& pred) : CellAggregationAlgorihm<RegularCell>(0.0, pred) {}

  virtual RegularCell::entry_t_ptr applyTo(
      const RegularCell& cell) override;
};

} // namespace crownet
