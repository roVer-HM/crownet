/*
 * Cell.h
 *
 *  Created on: Nov 20, 2020
 *      Author: sts
 */

#pragma once

#include <map>
#include <memory>

#include <omnetpp/cexception.h>
#include <memory>

#include "crownet/common/Entry.h"
#include "crownet/dcd/generic/iterator/CellDataIterator.h"
#include "crownet/dcd/identifier/Identifiers.h"

namespace crownet {

/**
 * Container class for some region of space in which on node (owner)
 * collects measurements (IEntry<N, T> objects) and sorts them based
 * on origin.
 *
 * C = cell_key_t   is the type of the cell identifier
 *                  E.g. (X, Y, length, width), (Lat, Lon), ...
 * N = node_key_t   is the type of the node identifier owning this cell
 *                  E.g. OppId(:= int), MacAddress, String, ...
 * T = time_t       is the type of time used to place measurements in time
 *
 * IEntry<N, T>     is measurement object which has
 *   - a source/origin which is donated by a node_key_t value
 *   - a count (int) of nodes corresponding to this cell. Note IEntry does
 *     not hold a reference to the Cell to which the measurements belong.
 *     use Cell<C, N, T>::cell_value_type for this.
 *   - ...
 *
 * Cell<C, N, T> provides common iterators (all, valid) and accepts
 * a couple of visitors to mutate and retrieve information.
 *
 *
 */
template <typename C, typename N, typename T>
class Cell {
 public:
  using cell_key_t = C;
  using node_key_t = N;
  using time_t = T;

  using entry_t = IEntry<node_key_t, time_t>;
  using entry_t_ptr = std::shared_ptr<entry_t>;
  using entry_ctor_t = EntryDefaultCtorImpl<node_key_t, time_t>;

  using localEntry_t = ILocalEntry<node_key_t, time_t>; // the measurement created by the owner itself
  using localEntry_t_ptr = std::shared_ptr<localEntry_t>;

  using map_t = std::map<node_key_t, entry_t_ptr>;
  using value_type_const = typename map_t::value_type;
  using value_type = std::pair<node_key_t, entry_t_ptr>;
  //  using cell_type = std::pair<cell_key_t, entry_t_ptr>;

 public:
  virtual ~Cell() = default;
  Cell() {}
  Cell(cell_key_t cell_id, node_key_t owner_id)
      : cell_id(cell_id), owner_id(owner_id) {}

  // getter
  map_t& getData() { return data; }
  const map_t& getData() const;
  bool hasData(const node_key_t& node_id) const;
  bool hasLocal() const;
  bool hasValid() const;
  bool hasValidLocal() const;
  localEntry_t_ptr getLocal();
  localEntry_t_ptr getLocal() const;
  entry_t_ptr get(const node_key_t node_id);
  entry_t_ptr const get(const node_key_t node_id) const;
  const cell_key_t& getCellId() const { return cell_id; }
  const node_key_t& getOwnerId() const { return owner_id; }
  entry_t_ptr val() { return cell_value; }  // selected/calculated value
  const time_t lastSent(){ return last_sent; }

  // setter
  void put(entry_t_ptr&& m);
  void put(entry_t_ptr& m);
  void setCellId(cell_key_t _cell_id) { cell_id = _cell_id; }
  void setOwnerId(node_key_t _owner_id) { owner_id = _owner_id; }
  void incrementLocal(const node_key_t& countedNodeId,
                      const time_t& time,
                      const double& value = 1.0);
  void sentAt(const time_t& time);


  /**
   *  A Cell<C, N, T> object acts as an iterator for its data items.
   *  The standard begin()/end() iterators will iterate over _all_
   *  items stored in the Cell.
   *
   */
  // _all_ items in data
  typename map_t::iterator begin() { return data.begin(); }
  typename map_t::iterator end() { return data.end(); }
  const typename map_t::iterator begin() const;
  const typename map_t::iterator end() const;
  // _ONLY_ valid items in data where entry->valid() returns true.
  CellDataIterator<Cell<C, N, T>> validIter();
  CellDataIterator<Cell<C, N, T>> validIter() const;

  /**
   *  Visitors
   *  Expect some kind of callable which accepts a Cell<C,N,T>& reference.
   *  Possible solutions are lambdas [](Cell<C,N,T>& c){} or
   *  classes with an overloaded operator()(Cell<C,N,T>& c).
   *
   *  See CellVisitors.h for examples (e.g. GetCellVisitor and SetCellVisitor)
   *  Look at std::bind to use a member function as a visitor.
   */
  // generic visitor
  template <typename Fn, typename Ret>
  Ret accept(Fn visitor);
  template <typename Fn, typename Ret>
  Ret accept(const Fn visitor) const;
  // common specialized visitors (cleaner calls without to many <...> )
  template <typename Fn>
  void acceptSet(Fn visitor);
  template <typename Fn>
  entry_t_ptr acceptGetEntry(const Fn visitor) const;
  template <typename Fn>
  bool acceptCheck(const Fn visitor) const;
  // compute value of cell based on computeAlg
  template <typename Fn>
  void computeValue(const Fn computeAlg);
  entry_t_ptr createEntry(const double count) const;

  // display
  std::string str() const;
  std::string infoCompact() const;

  // operators
  // Sort order cell_key_t --> node_key_t
  // equality is based on cell_key_t and node_key_t **only**. The state of
  // data is not looked at.
  bool operator<(const Cell<C, N, T>& rhs) const;
  bool operator>(const Cell<C, N, T>& rhs) const;
  bool operator==(const Cell<C, N, T>& rhs) const;

 private:
  map_t data;
  cell_key_t cell_id;
  node_key_t owner_id;
  entry_ctor_t entryCtor;
  entry_t_ptr cell_value;  //  selected or calculated value.
  time_t last_sent; // time at which the cell_value was last broadcasted.
};

#include "Cell.tcc"

}  // namespace crownet
