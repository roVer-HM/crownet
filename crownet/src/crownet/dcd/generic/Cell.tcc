/*
 * cell.tcc
 *
 *  Created on: Nov 21, 2020
 *      Author: sts
 */

#pragma once
#include "crownet/dcd/generic/Cell.h"

///////////////////////////////////////////////////////////////////////////////
/// Cell<C, N, T>   ///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

template <typename C, typename N, typename T>
std::ostream& operator<<(std::ostream& os, const Cell<C, N, T>& cell) {
  return os << cell.str();
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::operator<(const Cell<C, N, T>& rhs) const {
  return this->cell_id < rhs.cell_id ||
         (!(rhs.cell_id < this->cell_id) && this->owner_id < rhs.owner_id);
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::operator>(const Cell<C, N, T>& rhs) const {
  return !this->operator>(rhs) && !this->operator==(rhs);
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::operator==(const Cell<C, N, T>& rhs) const {
  return this->cell_id == rhs.cell_id && this->owner_id == rhs.owner_id;
}

template <typename C, typename N, typename T>
const typename Cell<C, N, T>::map_t& Cell<C, N, T>::getData() const {
  return const_cast<Cell<C, N, T>*>(this)->getData();
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::hasData(const node_key_t& node_id) const {
  return this->data.find(node_id) != this->data.end();
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::hasLocal() const {
  return hasData(this->owner_id);
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::hasValid() const {
  return this->validIter() != this->validIter().end();
}

template <typename C, typename N, typename T>
bool Cell<C, N, T>::hasValidLocal() const {
  return this->hasLocal() && getLocal()->valid();
}

template <typename C, typename N, typename T>
void Cell<C, N, T>::put(entry_t_ptr&& m) {
  auto& key = m->getSource();
  this->data[key] = std::move(m);
}

template <typename C, typename N, typename T>
void Cell<C, N, T>::put(entry_t_ptr& m) {
  auto& key = m->getSource();
  this->data[key] = m;
}

template <typename C, typename N, typename T>
typename Cell<C, N, T>::localEntry_t_ptr Cell<C, N, T>::getLocal() {
  auto entry = this->get(this->owner_id);
  return std::static_pointer_cast<ILocalEntry<N, T>>(entry);
}

template <typename C, typename N, typename T>
typename Cell<C, N, T>::localEntry_t_ptr Cell<C, N, T>::getLocal() const {
  return const_cast<Cell<C, N, T>*>(this)->getLocal();
}

template <typename C, typename N, typename T>
typename Cell<C, N, T>::entry_t_ptr Cell<C, N, T>::get(
    const node_key_t node_id) {
  if (!hasData(node_id))
    throw omnetpp::cRuntimeError("node_id %s not found in cell %s",
                                 node_id.str().c_str(),
                                 this->cell_id.str().c_str());
  return this->data[node_id];
}

template <typename C, typename N, typename T>
typename Cell<C, N, T>::entry_t_ptr const Cell<C, N, T>::get(
    const node_key_t node_id) const {
  return const_cast<Cell<C, N, T>*>(this)->get(node_id);
}

template <typename C, typename N, typename T>
const typename Cell<C, N, T>::map_t::iterator Cell<C, N, T>::begin() const {
  return const_cast<Cell<C, N, T>*>(this)->data.begin();
}

template <typename C, typename N, typename T>
const typename Cell<C, N, T>::map_t::iterator Cell<C, N, T>::end() const {
  return const_cast<Cell<C, N, T>*>(this)->data.end();
}

template <typename C, typename N, typename T>
CellDataIterator<Cell<C, N, T>> Cell<C, N, T>::validIter() {
  return CellDataIterator<Cell<C, N, T>>::ValidDataIter(this);
}

template <typename C, typename N, typename T>
CellDataIterator<Cell<C, N, T>> Cell<C, N, T>::validIter() const {
  return const_cast<Cell<C, N, T>*>(this)->validIter();
}

template <typename C, typename N, typename T>
template <typename Fn>
void Cell<C, N, T>::acceptSet(Fn visitor) {
  visitor(*this);
}

template <typename C, typename N, typename T>
template <typename Fn>
typename Cell<C, N, T>::entry_t_ptr Cell<C, N, T>::acceptGetEntry(
    const Fn visitor) const {
  return visitor(*this);
}

template <typename C, typename N, typename T>
template <typename Fn>
void Cell<C, N, T>::computeValue(const Fn computeAlg) {
  this->cell_value = computeAlg(*this);  // may set empty shared_ptr
  if (this->cell_value) this->cell_value->setSelectedIn(computeAlg.getName());
}

template <typename C, typename N, typename T>
template <typename Fn>
bool Cell<C, N, T>::acceptCheck(const Fn visitor) const {
  return visitor(*this);
}

template <typename C, typename N, typename T>
template <typename Fn, typename Ret>
Ret Cell<C, N, T>::accept(Fn visitor) {
  return visitor(*this);
}

template <typename C, typename N, typename T>
template <typename Fn, typename Ret>
Ret Cell<C, N, T>::accept(const Fn visitor) const {
  return visitor(*this);
}

template <typename C, typename N, typename T>
std::string Cell<C, N, T>::str() const {
  std::stringstream os;
  os << "{cell_id: " << this->cell_id << " owner_id: " << this->owner_id << "}";
  return os.str();
}

template <typename C, typename N, typename T>
void Cell<C, N, T>::incrementLocal(const node_key_t& countedNodeId,
                                   const time_t& time) {
  if (!hasData(this->owner_id)) {
    // create local entry and set cell owner as source.
    auto e = this->entryCtor.localEntry();
    e->setSource(this->owner_id);
    this->data[this->owner_id] = e;
  }
  std::shared_ptr<ILocalEntry<N, T>> lEntry =
      std::static_pointer_cast<ILocalEntry<N, T>>(this->data[this->owner_id]);
  auto ret = lEntry->nodeIds.insert(countedNodeId);
  if (ret.second) {
    // new node inserted
    lEntry->incrementCount(time);
  } else {
    // node already exists just update time.
    lEntry->touch(time);
  }
}
