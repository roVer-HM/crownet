/*
 * CellVisitors.h
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include <omnetpp.h>

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
  virtual ~CellVisitor() = default;
  virtual Ret applyTo(C& cell) = 0;
  Ret operator()(C& cell) { return this->applyTo(cell); }
  virtual std::string getVisitorName() const { return ""; };
};

template <typename C, typename Ret>
class ConstCellVisitor {
 public:
  virtual ~ConstCellVisitor() = default;
  virtual Ret applyTo(const C& cell) const = 0;
  Ret operator()(const C& cell) const { return this->applyTo(cell); }
  virtual std::string getVisitorName() const { return ""; };
};


template<typename C, typename T>
class TimestampMixin : public C {
public:
    TimestampMixin() : C(), time() {}
    TimestampMixin(const TimestampMixin& other) : C(other) {this->time = other.getTime();}

    void setTime(const T& t){
        this->time = t;
    }
    const T& getTime() const {
        return this->time;
    }

private:
    T time;
};

template<typename C, typename Map>
class MapMixin : public C {
public:
    MapMixin() : C() {}
    MapMixin(const MapMixin& other): C(other) {this->map = other.getMap();}

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
  virtual typename C::entry_t_ptr applyTo(const C& cell) const = 0;
};

template <typename C>
class TimestampedGetEntryVisitor : public GetEntryVisitor<C>, public Timestamped<C>  {
 public:
  TimestampedGetEntryVisitor(typename C::time_t time): Timestamped<C>(time) {}
  TimestampedGetEntryVisitor(const TimestampedGetEntryVisitor<C>& other) {this->setTime(other.getTime());}
  virtual typename C::entry_t_ptr applyTo(const C& cell) const = 0;
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
  virtual void applyTo(C& cell) {throw omnetpp::cRuntimeError("Implement me!");};
};

template <typename C>
class TimestampedVoidCellVisitor : public VoidCellVisitor<C>, public Timestamped<C> {
 public:
    TimestampedVoidCellVisitor() : VoidCellVisitor<C>(), Timestamped<C>() {}
    TimestampedVoidCellVisitor(typename C::time_t time) :  Timestamped<C>(time) {}
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
  virtual bool applyTo(const C& cell) const = 0;
};

template <typename C>
class CellPrinterAll : public ConstCellVisitor<C, std::string> {
 public:
  CellPrinterAll(int indent) : indnet(indent) {}
  CellPrinterAll() : indnet(0) {}

  virtual std::string applyTo(const C& cell) const override {
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
  virtual std::string applyTo(const C& cell) const override {
    std::stringstream os;
    os << cell.str() << "(valid only)" << std::endl;
    for (const auto e : cell.validIter()) {
      os << "  " << e.first << " --> " << e.second->str() << std::endl;
    }
    return os.str();
  }
};

}  // namespace crownet
