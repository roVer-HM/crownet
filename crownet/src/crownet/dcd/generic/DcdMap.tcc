/*
 * dcd_map.tcc
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#pragma once
#include "crownet/dcd/generic/DcdMap.h"

template <typename C, typename N, typename T>
const typename DcDMap<C, N, T>::map_t::iterator DcDMap<C, N, T>::begin() const {
  return const_cast<DcDMap<C, N, T>*>(this)->cells.begin();
}

template <typename C, typename N, typename T>
const typename DcDMap<C, N, T>::map_t::iterator DcDMap<C, N, T>::end() const {
  return const_cast<DcDMap<C, N, T>*>(this)->cells.end();
}

template <typename C, typename N, typename T>
DcDMapIterator<DcDMap<C, N, T>> DcDMap<C, N, T>::validLocal() {
  return DcDMapIterator<DcDMap<C, N, T>>::ValidLocalCellIter(this);
}

template <typename C, typename N, typename T>
DcDMapIterator<DcDMap<C, N, T>> DcDMap<C, N, T>::validLocal() const {
  return const_cast<DcDMap<C, N, T>*>(this)->validLocal();
}

template <typename C, typename N, typename T>
DcDMapIterator<DcDMap<C, N, T>> DcDMap<C, N, T>::allLocal() {
  return DcDMapIterator<DcDMap<C, N, T>>::LocalCellIter(this);
}

template <typename C, typename N, typename T>
DcDMapIterator<DcDMap<C, N, T>> DcDMap<C, N, T>::allLocal() const {
  return const_cast<DcDMap<C, N, T>*>(this)->allLocal();
}

template <typename C, typename N, typename T>
DcDMapIterator<DcDMap<C, N, T>> DcDMap<C, N, T>::valid() {
  return DcDMapIterator<DcDMap<C, N, T>>::ValidCellIter(this);
}

template <typename C, typename N, typename T>
DcDMapIterator<DcDMap<C, N, T>> DcDMap<C, N, T>::valid() const {
  return const_cast<DcDMap<C, N, T>*>(this)->valid();
}

template <typename C, typename N, typename T>
const int DcDMap<C, N, T>::validCellCount() const{
    auto iter = valid();
    int i=0;
    for(const auto& e: iter){++i;}
    return i;
}

template <typename C, typename N, typename T>
const int DcDMap<C, N, T>::validLocalCellCount() const{
    auto iter = validLocal();
    int i=0;
    while(iter != iter.end()){
        ++i;
        ++iter;
    }
    return i;
}

template <typename C, typename N, typename T>
const int DcDMap<C, N, T>::allLocalCellCount() const{
    auto iter = allLocal();
    int i=0;
    while(iter != iter.end()){
        ++i;
        ++iter;
    }
    return i;
}


template <typename C, typename N, typename T>
const typename DcDMap<C, N, T>::map_t* DcDMap<C, N, T>::getCells() const {
  return const_cast<DcDMap<C, N, T>*>(this)->getCells();
}

template <typename C, typename N, typename T>
std::map< typename DcDMap<C, N, T>::node_key_t,  typename DcDMap<C, N, T>::cell_key_t>&  DcDMap<C, N, T>::getNeighborhood(){
    return this->neighborhood;
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::setOwnerCell(const traci::TraCIPosition& pos) {
  this->setOwnerCell(this->cellKeyProvider->getCellKey(pos));
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::setCellKeyProvider(
    std::shared_ptr<CellKeyProvider<C>> provider) {
  this->cellKeyProvider = provider;
}

template <typename C, typename N, typename T>
template <typename Fn>
void DcDMap<C, N, T>::visitCells(Fn visitor) {
  for (auto& entry : this->cells) {
    entry.second.acceptSet(visitor);
  }
}

template <typename C, typename N, typename T>
template <typename Fn>
void DcDMap<C, N, T>::computeValues(Fn visitor) {
  // only compute values if needed. Ensure that first computation ( lastComputedAt == ZERO) takes place
  if (lastComputedAt < this->timeProvider->now() || lastComputedAt == this->timeProvider->zero()) {
    for (auto& entry : this->cells) {
      entry.second.computeValue(visitor);
    }
    lastComputedAt = this->timeProvider->now();
  }
}

template <typename C, typename N, typename T>
template <typename Fn>
void DcDMap<C, N, T>::applyVisitorTo(const cell_key_t& cell_id, Fn visitor) {
  this->getCell(cell_id).acceptSet(visitor);
}

template <typename C, typename N, typename T>
bool DcDMap<C, N, T>::hasCell(const cell_key_t& cell_id) const {
  return this->cells.find(cell_id) != this->cells.end();
}

template <typename C, typename N, typename T>
bool DcDMap<C, N, T>::hasDataFrom(const cell_key_t& cell_id,
                                  const node_key_t& owner_id) const {
  auto iter = this->cells.find(cell_id);
  if (iter == this->cells.end()) return false;
  return iter->second.hasData(owner_id);
}

template <typename C, typename N, typename T>
std::string DcDMap<C, N, T>::str() const {
  std::stringstream os;
  os << "{map_owner: " << this->owner_id
     << " cell_count: " << this->cells.size()
     << " local_cell_count: " << this->allLocalCellCount() << "}";
  return os.str();
}

template <typename C, typename N, typename T>
std::string DcDMap<C, N, T>::strFull() const {
  CellPrinterAll<Cell<C, N, T>> printAll{2};  // indent
  std::stringstream os;
  os << this->str() << std::endl;
  for (const auto& val : this->cells) {
    os << val.second
              .template accept<CellPrinterAll<Cell<C, N, T>>, std::string>(
                  printAll);
  }
  return os.str();
}

template <typename C, typename N, typename T>
typename DcDMap<C, N, T>::cell_t& DcDMap<C, N, T>::getCell(
    const cell_key_t& cell_id) {
  if (!hasCell(cell_id)) return this->createCell(cell_id);
  return this->cells[cell_id];
}

template <typename C, typename N, typename T>
typename DcDMap<C, N, T>::cell_t& DcDMap<C, N, T>::createCell(
    const cell_key_t& cell_id) {
  auto entry = this->cells.emplace(
      std::piecewise_construct, std::forward_as_tuple(cell_id),
      std::forward_as_tuple(cell_t(cell_id, this->getOwnerId())));
  this->cellKeyStream->addNew(cell_id, this->timeProvider->now());
  return entry.first->second;
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::update(const cell_key_t& cell_id,
                             typename cell_t::entry_t_ptr&& m_data) {
  auto& cell = this->getCell(cell_id);
  cell.put(std::move(m_data));
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::update(const cell_key_t& cell_id,
                             typename cell_t::entry_t_ptr& m_data) {
  auto& cell = this->getCell(cell_id);
  cell.put(m_data);
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::incrementLocal(const traci::TraCIPosition& pos,
                                     const node_key_t sourceNodeId,
                                     const time_t time) {
  auto cellId = this->cellKeyProvider->getCellKey(pos);
  auto& cell = this->getCell(cellId);

  // 0) each node must be added only once to the local map
  if (this->isInNeighborhood(sourceNodeId)) {
    throw omnetpp::cRuntimeError(
        "duplicate nodeId [%s] in neighbourhood found.",
        sourceNodeId.str().c_str());
  }

  // 1) increment local count in cell [in Cell]
  // 2) add nodeId into nodeId set [in Cell]
  cell.incrementLocal(sourceNodeId, time);

  // 3) add mapping nodeId->cellId into updateNeighbourCell [in Map]
  this->addToNeighborhood(sourceNodeId, cellId);
}

template <typename C, typename N, typename T>
bool DcDMap<C, N, T>::isInNeighborhood(const node_key_t& neigbourId) const {
  return this->neighborhood.find(neigbourId) != this->neighborhood.end();
}

template <typename C, typename N, typename T>
int DcDMap<C, N, T>::sizeOfNeighborhood() const {
  return this->neighborhood.size();
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::clearNeighborhood() {
  this->neighborhood.clear();
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::removeFromNeighborhood(const node_key_t& neigborId) {
  this->neighborhood.erase(neigborId);
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::moveNeighborTo(const node_key_t& neigborId,
                                     const cell_key_t& cellId) {
  if (!this->isInNeighborhood(neigborId)) {
    throw omnetpp::cRuntimeError("Expected node_id: [%s] in neighbourhood",
                                 neigborId.str().c_str());
  }
  this->neighborhood[neigborId] = cellId;
}

template <typename C, typename N, typename T>
void DcDMap<C, N, T>::addToNeighborhood(const node_key_t& neigborId,
                                        const cell_key_t& cellId) {
  this->neighborhood[neigborId] = cellId;
}

template <typename C, typename N, typename T>
typename DcDMap<C, N, T>::cell_key_t DcDMap<C, N, T>::getNeighborCell(
    const node_key_t& neigborId) {
  if (!this->isInNeighborhood(neigborId)) {
    throw omnetpp::cRuntimeError("Expected node_id: [%s] in neighbourhood",
                                 neigborId.str().c_str());
  }
  return this->neighborhood[neigborId];
}
