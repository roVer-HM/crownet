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
typename Cell<C, N, T>::entry_t_ptr Cell<C, N, T>::getLocal() {
  return this->get(this->owner_id);
}

template <typename C, typename N, typename T>
typename Cell<C, N, T>::entry_t_ptr Cell<C, N, T>::getLocal() const {
  return const_cast<Cell<C, N, T>*>(this)->getLocal();
}

template <typename C, typename N, typename T>
template <typename E>
std::shared_ptr<E> Cell<C, N, T>::get(
    const node_key_t node_id) {
  if (!hasData(node_id))
    throw omnetpp::cRuntimeError("node_id %s not found in cell %s",
                                 node_id.str().c_str(),
                                 this->cell_id.str().c_str());
  return std::dynamic_pointer_cast<E>(this->data[node_id]);
}

template <typename C, typename N, typename T>
template <typename E>
std::shared_ptr<E> const Cell<C, N, T>::get(
    const node_key_t node_id) const {
  return const_cast<Cell<C, N, T>*>(this)->get<E>(node_id);
}

template <typename C, typename N, typename T>
template <typename E>
std::shared_ptr<E> Cell<C, N, T>::get() {
    return get<E>(owner_id);
}

template <typename C, typename N, typename T>
template <typename E>
std::shared_ptr<E> const Cell<C, N, T>::get() const {
  return const_cast<Cell<C, N, T>*>(this)->get<E>(owner_id);
}



template <typename C, typename N, typename T>
template <typename E>
std::shared_ptr<E> Cell<C, N, T>::getOrCreate(const node_key_t node_id){
    if (!hasData(node_id)){
        auto e = std::make_shared<E>();
        e->setSource(node_id);
        this->data[node_id] = e;
    }
    return get<E>(node_id);
}
template <typename C, typename N, typename T>
template <typename E>
std::shared_ptr<E> Cell<C, N, T>::getOrCreate(){
    return getOrCreate<E>(owner_id);
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
void Cell<C, N, T>::acceptSet(Fn* visitor) {
  visitor->operator()(*this);
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
  // 1 clear selection flag on all entries
  for(auto & e : this->data){
      e.second->setSelectedIn("");
  }
  // 2 apply computeAlg to select or compute value which represents this cell.
  this->cell_value = computeAlg->operator()(*this);  // may set empty shared_ptr
  // 3 set selection flag
  if (this->cell_value) this->cell_value->setSelectedIn(computeAlg->getVisitorName());
  // todo: if computeAlg creates a new entry (i.e compute not select) this->data
  // does not contain the selected value!
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
std::string Cell<C, N, T>::infoCompact() const{
    std::stringstream os;
    os << "[#" << this->data.size();
    int validCount = 0;
    for (const auto e : this->validIter()) {
        ++validCount;
    }
    os << "V" << validCount;
    if (this->hasLocal()){
        const auto l = this->getLocal();
        std::string valid = l->valid() ? "" : "-";
        os << "] -> [L" << l->getSource() << ":" << valid << l->getCount() << "|" << l->getMeasureTime().ustr() << "]";
    } else {
        os << "] -> ";
    }
    for(auto const& e : this->data){
        if(e.first != this->owner_id){
            std::string valid2 = e.second->valid() ? "" : "-";
            os << "[" << e.second->getSource() << ":" << valid2 << e.second->getCount() << "|" << e.second->getMeasureTime().ustr() << "]";
        }
    }

    return os.str();
}

template <typename C, typename N, typename T>
typename Cell<C, N, T>::entry_t_ptr  Cell<C, N, T>::createEntry(const double count) const{
    auto e = this->entryCtor.entry();
    e->setCount(count);
    return e;
}

template <typename C, typename N, typename T>
void Cell<C, N, T>::sentAt(const time_t& time){
    this->last_sent = time;
}
