/*
 * CellVisitors.h
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include <omnetpp.h>
#include <functional>

namespace crownet {

/**
 * Generic C = cell visitor to mutate / change the cell state
 *
 * This is the base class to use with Cell<>::accept<Visitor,
 * ReturnType>(visitor) See convenient SetCellVisitor and
 * Cell<>::acceptSet(visitor) for common usages.
 */
template <typename C, typename Ret>
class CellVisitor : public omnetpp::cObject{
 public:
  using cell_t = C;
  virtual ~CellVisitor() = default;
  CellVisitor(): omnetpp::cObject(), entryPredicate(CellDataIterator<C>::getAllDataIter_pred()) {}
  CellVisitor(typename CellDataIterator<C>::pred_t& pred): omnetpp::cObject(), entryPredicate(pred){}
  CellVisitor(const CellVisitor&) = delete; // no-copy pass visitors as pointer/references

  virtual Ret applyTo(C& cell) = 0;
  Ret operator()(C& cell) { return this->applyTo(cell); }
  virtual std::string getVisitorName() const { return ""; };

  virtual void setCellIterPredicate(typename CellDataIterator<C>::pred_t& pred) {entryPredicate = pred;}
  CellDataIterator<C> cellIter(C& cell) {return cell.iterBy(entryPredicate); }
  const CellDataIterator<C> cellIter(const C& cell) const {return const_cast<CellVisitor<C, Ret>*>(this)->cellIter(cell);}

 protected:
      typename CellDataIterator<C>::pred_t entryPredicate;

};

template <typename C, typename Ret>
class ConstCellVisitor {
 public:
  virtual ~ConstCellVisitor() = default;
  ConstCellVisitor() : entryPredicate(CellDataIterator<C>::getAllDataIter_pred()){}
  ConstCellVisitor(typename CellDataIterator<C>::pred_t& pred): entryPredicate(pred){}
  ConstCellVisitor(typename CellDataIterator<C>::pred_t&& pred): ConstCellVisitor(pred){} // allow move
  ConstCellVisitor(const ConstCellVisitor&) = delete; // no-copy pass visitors as pointer/references

  virtual Ret applyTo(const C& cell) = 0;
  Ret operator()(const C& cell) { return this->applyTo(cell); }
  virtual std::string getVisitorName() const { return ""; };

  virtual void setCellIterPredicate(typename CellDataIterator<C>::pred_t& pred) {entryPredicate = pred;}
  CellDataIterator<C> cellIter(C& cell) {return cell.iterBy(entryPredicate); }
  const CellDataIterator<C> cellIter(const C& cell) const {return cell.iterBy(entryPredicate);}


 protected:
      typename CellDataIterator<C>::pred_t entryPredicate;

};


template<typename C>
class TimestampMixin : public C {
public:
    TimestampMixin() : C(), time() {}
//    TimestampMixin(const TimestampMixin& other) : C(other) {this->time = other.getTime();}

    void setTime(const typename C::cell_t::time_t& t){
        this->time = t;
    }
    const typename C::cell_t::time_t& getTime() const {
        return this->time;
    }

private:
    typename C::cell_t::time_t time;
};

template<typename C>
class IdempotenceMixin: public TimestampMixin<C>{
public:
    IdempotenceMixin(): TimestampMixin<C>() {}

    void setLastCall(const typename C::time_t& t){
        this->lastTime = t;
    }
    const bool checkLastCall() const {
        return this->time > this->lastTime;
    }


protected:
 typename C::time_t lastTime;
};


template<typename C, typename Map>
class MapMixin : public C {
public:
    MapMixin() : C() {}

    void setMap(std::shared_ptr<Map> map) { this->map = map;}
    std::shared_ptr<Map> getMap() { return this->map;}
    std::shared_ptr<Map> getMap() const { return this->map;}

private:
    std::shared_ptr<Map> map;

};





template <typename C>
class Timestamped{
 public:
    Timestamped() : time() {}
    Timestamped(typename C::time_t time) : time(time) {}
    void setTime(const typename C::time_t& t){
        this->time = t;
    }
    const typename C::time_t& getTime() const {
        return this->time;
    }

 protected:
  typename C::time_t time;
};

template <typename C>
class IdenpotanceTimestamped : public  Timestamped<C>{
 public:
    IdenpotanceTimestamped() : Timestamped<C>() {}
    IdenpotanceTimestamped(typename C::time_t time) : Timestamped<C>(time) {}
    void setLastCall(const typename C::time_t& t){
        this->lastTime = t;
    }
    typename C::time_t getLastCallTime() const {return lastTime;}
    const bool checkDoUpdate() {
        if (first_call){
            first_call = true;
            return true;
        }
        return this->time > this->lastTime;
    }

 protected:
  typename C::time_t lastTime;
  bool first_call = true;
};


/**
 * Convenient Visitor for returning value_type !! const !!
 *
 * Generic C = Cell visitor returning a matching value_type (key/value-pair)
 * contained in the data of the Cell object.
 *
 * The Visitor has access to the Cell object (see Cell.h)
 *
 * Use object of this type for Cell::acceptGet(Fn visitor) acceptors
 * or provide any other callable with this signature see Visitors in Cell.h
 */
template <typename C>
class GetEntryVisitor : public ConstCellVisitor<C, typename C::entry_t_ptr> {
 public:
    GetEntryVisitor(): ConstCellVisitor<C, typename C::entry_t_ptr>() {}
    GetEntryVisitor(typename CellDataIterator<C>::pred_t& pred): ConstCellVisitor<C, typename C::entry_t_ptr>(pred){}


  virtual typename C::entry_t_ptr applyTo(const C& cell) = 0;
};

template <typename C>
class TimestampedGetEntryVisitor : public GetEntryVisitor<C>, public Timestamped<C>  {
 public:
  TimestampedGetEntryVisitor(typename C::time_t time): GetEntryVisitor<C>(), Timestamped<C>(time) {}
  TimestampedGetEntryVisitor(typename C::time_t time, typename CellDataIterator<C>::pred_t& pred): GetEntryVisitor<C>(pred), Timestamped<C>(time) {}
  TimestampedGetEntryVisitor(typename C::time_t time, typename CellDataIterator<C>::pred_t&& pred): TimestampedGetEntryVisitor(time, pred) {} //allow move

};


/**
 * Convenient visitor for mutating calls without return values
 * Convenient visitor for mutating calls without return values (+ timestamp)
 *
 * Generic C = Cell visitor used to mutate / change data contained in
 * the Cell object.
 *
 * Does not return anything.
 *
 * Use object of this type for Cell::acceptSet(Fn visitor) acceptors
 * or provide any other callable with this signature see Visitors in Cell.h
 */
template <typename C>
class VoidCellVisitor : public CellVisitor<C, void> {
 public:
    VoidCellVisitor(): CellVisitor<C, void>() {}
    VoidCellVisitor(typename CellDataIterator<C>::pred_t& pred): CellVisitor<C, void>(pred){}
    virtual void applyTo(C& cell) {throw omnetpp::cRuntimeError("Implement me!");};
};

template <typename C>
class TimestampedVoidCellVisitor : public VoidCellVisitor<C>, public Timestamped<C> {
 public:
    TimestampedVoidCellVisitor() : VoidCellVisitor<C>(), Timestamped<C>() {}
    TimestampedVoidCellVisitor(typename C::time_t time) : VoidCellVisitor<C>(), Timestamped<C>(time) {}
    TimestampedVoidCellVisitor(typename C::time_t time, typename CellDataIterator<C>::pred_t& pred) : VoidCellVisitor<C>(pred), Timestamped<C>(time) {}

};

template <typename C>
class IdenpotanceTimestampedVoidCellVisitor : public VoidCellVisitor<C>, public IdenpotanceTimestamped<C> {
 public:
    IdenpotanceTimestampedVoidCellVisitor() : VoidCellVisitor<C>(), IdenpotanceTimestamped<C>() {}
    IdenpotanceTimestampedVoidCellVisitor(typename C::time_t time) :  IdenpotanceTimestamped<C>(time) {}
    IdenpotanceTimestampedVoidCellVisitor(typename C::time_t time, typename CellDataIterator<C>::pred_t& pred)
        : VoidCellVisitor<C>(pred), IdenpotanceTimestamped<C>(time) {}
    IdenpotanceTimestampedVoidCellVisitor(typename C::time_t time, typename CellDataIterator<C>::pred_t&& pred)
        : IdenpotanceTimestampedVoidCellVisitor(time, pred){}// allow move


 public:
    virtual void applyTo(C& cell) override {
        if (IdenpotanceTimestamped<C>::checkDoUpdate()){
            applyIfChanged(cell);
        }
    }
    virtual void applyIfChanged(C& cell)=0;

};

template <typename C>
class IdempotanceTimestampedGetCellVisitor : public GetEntryVisitor<C>, public IdenpotanceTimestamped<C> {
 public:
    IdempotanceTimestampedGetCellVisitor() : GetEntryVisitor<C>(), IdenpotanceTimestamped<C>() {}
    IdempotanceTimestampedGetCellVisitor(typename C::time_t time) : GetEntryVisitor<C>(), IdenpotanceTimestamped<C>(time) {}
    IdempotanceTimestampedGetCellVisitor(typename C::time_t time, typename CellDataIterator<C>::pred_t& pred) : GetEntryVisitor<C>(pred), IdenpotanceTimestamped<C>(time) {}


 public:
    virtual typename C::entry_t_ptr applyTo(const C& cell) override {
        if (IdenpotanceTimestamped<C>::checkDoUpdate()){
            return applyIfChanged(cell);
        }
        return cell.val();
    }
    virtual typename C::entry_t_ptr applyIfChanged(const C& cell) const = 0;

};

template <typename C>
class CellAggregationAlgorihm:  public IdempotanceTimestampedGetCellVisitor<C> {
public:
    CellAggregationAlgorihm(): IdempotanceTimestampedGetCellVisitor<C>() {}
    CellAggregationAlgorihm(typename C::time_t time): IdempotanceTimestampedGetCellVisitor<C>(time) {}
    CellAggregationAlgorihm(typename C::time_t time, typename CellDataIterator<C>::pred_t& pred): IdempotanceTimestampedGetCellVisitor<C>(time, pred) {}
    CellAggregationAlgorihm(typename C::time_t time, typename CellDataIterator<C>::pred_t&& pred): IdempotanceTimestampedGetCellVisitor<C>(time, pred) {} //allow move

public:

    virtual typename C::entry_t_ptr update_selection(typename C::entry_t_ptr val, bool copy = true){
        val->setSelectedIn(this->getVisitorName());
        if (copy){
            return std::make_shared<typename C::entry_t>(*val);
        }
        return val;
    }
};


/**
 * Convenient visitor for const checks returning booleans
 *
 * Generic C = Cell visitor to answer a given predicate using
 * the Cell object.
 *
 * Does return bool.
 *
 * Use object of this type for Cell::acceptCheck(Fn visitor) acceptors
 * or provide any other callable with this signature see Visitors in Cell.h
 */
template <typename C>
class CheckCellVisitor : public ConstCellVisitor<C, bool> {
 public:
   CheckCellVisitor(): ConstCellVisitor<C, bool>() {}
   CheckCellVisitor(typename CellDataIterator<C>::pred_t& pred): ConstCellVisitor<C, bool>(pred){}

  virtual bool applyTo(const C& cell)  = 0;
};

template <typename C>
class ConstLambdaVisitor: public ConstCellVisitor<C, void> {
public:
    using func_t = std::function<void(const  C&)>;
    ConstLambdaVisitor(func_t func, typename CellDataIterator<C>::pred_t& pred) : ConstCellVisitor<C, void>(pred), func(func) {}
    ConstLambdaVisitor(func_t func) : ConstCellVisitor<C, void>(), func(func) {}
    ConstLambdaVisitor() : ConstCellVisitor<C, void>(), func([](const C& cell){return;}) {} // do nothing lambda

    virtual void applyTo(const C& cell)  override {
        func(cell);
    }

private:
    func_t func;
};

template <typename C>
class CellPrinterAll : public ConstCellVisitor<C, std::string> {
 public:
  CellPrinterAll(int indent) : ConstCellVisitor<C, std::string>(), indnet(indent) {}
  CellPrinterAll() : ConstCellVisitor<C, std::string>(), indnet(0) {}

  virtual std::string applyTo(const C& cell)  override {
    std::stringstream os;
    os << std::string(indnet, ' ') << cell.str() << std::endl;
    for (const auto e : cell) {
      os << std::string(indnet + 2, ' ') << e.first << " --> "
         << e.second->str() << std::endl;
    }
    return os.str();
  }

 private:
  int indnet;
};

template <typename C>
class CellPrinterValid : public ConstCellVisitor<C, std::string> {
 public:
   CellPrinterValid() : ConstCellVisitor<C, std::string>(CellDataIterator<C>::getValidDataIter_pred()) {}

  virtual std::string applyTo(const C& cell)  override {
    std::stringstream os;
    os << cell.str() << "(valid only)" << std::endl;
    for (const auto e : cell.validIter()) {
      os << "  " << e.first << " --> " << e.second->str() << std::endl;
    }
    return os.str();
  }
};

}  // namespace crownet
