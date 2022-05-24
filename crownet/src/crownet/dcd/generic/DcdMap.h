/*
 * dcd_map.h
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include "crownet/dcd/generic/CellVisitors.h"  // *.tcc
#include "crownet/dcd/generic/iterator/DcDMapIterator.h"
#include "crownet/dcd/identifier/CellKeyProvider.h"
#include "crownet/dcd/identifier/TimeProvider.h"
#include "crownet/dcd/generic/ICellIdStream.h"

namespace crownet {

template <typename C, typename N, typename T>
class DcDMap {
 public:
  using cell_key_t = C;
  using node_key_t = N;
  using time_t = T;
  using cell_t = Cell<cell_key_t, node_key_t, time_t>;

  using map_t = std::map<cell_key_t, cell_t>;
  using cell_value_type_const = typename map_t::value_type;
  using cell_value_type = std::pair<node_key_t, cell_value_type_const>;

 public:
  virtual ~DcDMap() = default;
  DcDMap() {}
  DcDMap(node_key_t owner_id,
         std::shared_ptr<CellKeyProvider<C>> cellKeyProvider,
         std::shared_ptr<TimeProvider<T>> timeProvider,
         std::shared_ptr<ICellIdStream<C, N, T>> cellKeyStream)
      : owner_id(owner_id),
        cellKeyProvider(cellKeyProvider),
        timeProvider(timeProvider),
        cellKeyStream(cellKeyStream){
      this->cellKeyStream->setMap(this);
  }

  // getter
  const node_key_t& getOwnerId() const { return owner_id; }
  const cell_key_t& getOwnerCell() const { return owner_cell; }
  map_t& getCells() { return cells; }
  const map_t* getCells() const;
  bool hasCell(const cell_key_t& cell_id) const;
  bool hasDataFrom(const cell_key_t& cell_id, const node_key_t& owner_id) const;
  std::string str() const;
  std::string strFull() const;
  std::shared_ptr<CellKeyProvider<C>> getCellKeyProvider() const;
  std::map<node_key_t, cell_key_t>& getNeighborhood();

  // getter/setter (create if missing)
  cell_t& getCell(const cell_key_t& cell_id);
  cell_t& createCell(const cell_key_t& cell_id);

  // setter
  void setOwnerId(node_key_t _owner_id) { owner_id = _owner_id; }
  void setOwnerCell(cell_key_t _owner_cell) { owner_cell = _owner_cell; }
  void setOwnerCell(const traci::TraCIPosition& pos);
  void setCellKeyProvider(std::shared_ptr<CellKeyProvider<C>> provider);

  // map update methods
  void setEntry(const cell_key_t& cell_id, typename cell_t::entry_t_ptr&& m_data);
  void setEntry(const cell_key_t& cell_id, typename cell_t::entry_t_ptr& m_data);

  template <typename E = typename cell_t::entry_t>
  std::shared_ptr<E> getEntry(const cell_key_t& cell_id, const node_key_t& source);
  template <typename E = typename cell_t::entry_t>
  std::shared_ptr<E> getEntry(const cell_key_t& cell_id);
  template <typename E = typename cell_t::entry_t>
  std::shared_ptr<E> getEntry(const traci::TraCIPosition& pos);
  template <typename E = typename cell_t::entry_t>
  std::shared_ptr<E> getEntry(const traci::TraCIPosition& pos, const node_key_t& source);

  bool hasEntry(const cell_key_t& cell_id, const node_key_t& source);
  bool hasEntry(const cell_key_t& cell_id);
  bool hasEntry(const traci::TraCIPosition& pos, const node_key_t& source);
  bool hasEntry(const traci::TraCIPosition& pos);

  // iterators and visitors
  typename map_t::iterator begin() { return cells.begin(); }
  typename map_t::iterator end() { return cells.end(); }
  const typename map_t::iterator begin() const;
  const typename map_t::iterator end() const;
  DcDMapIterator<DcDMap<C, N, T>> validLocal();
  DcDMapIterator<DcDMap<C, N, T>> validLocal() const;
  DcDMapIterator<DcDMap<C, N, T>> allLocal();
  DcDMapIterator<DcDMap<C, N, T>> allLocal() const;
  DcDMapIterator<DcDMap<C, N, T>> valid();
  DcDMapIterator<DcDMap<C, N, T>> valid() const;
  const int validCellCount() const;
  const int validLocalCellCount() const;
  const int allLocalCellCount() const;


  template <typename Fn>
  void visitCells(Fn* visitor);


  template <typename Fn>
  void visitCells(Fn& visitor);

  template <typename Fn>
  void visitCells(Fn&& visitor);


  template <typename Fn>
  void computeValues(Fn visitor);
  template <typename Fn>
  void applyVisitorTo(const cell_key_t& cell_id, Fn visitor);

  std::shared_ptr<CellKeyProvider<C>> getCellKeyProvider() {return cellKeyProvider;}
  std::shared_ptr<ICellIdStream<C, N, T>> getCellKeyStream() {return cellKeyStream; }

 private:
  map_t cells;
  node_key_t owner_id;
  cell_key_t owner_cell; // owner position
  time_t lastComputedAt; // Zeitpunkt an dem der Wert berechnet wird
  std::shared_ptr<CellKeyProvider<C>> cellKeyProvider;
  std::shared_ptr<TimeProvider<T>> timeProvider;
  std::shared_ptr<ICellIdStream<C, N, T>> cellKeyStream;

 public:

  /*
   *  Manage neighborhood in local area.
   */
  bool isInNeighborhood(const node_key_t& neigbourId) const;
  int sizeOfNeighborhood() const;
  void clearNeighborhood();
  void removeFromNeighborhood(const node_key_t& neigborId);
  void removeFromNeighborhood(const cell_key_t& cellId);
  cell_key_t getNeighborCell(const node_key_t& neigborId);
  void moveNeighborTo(const node_key_t& neigbourId, const cell_key_t& cellId);
  void addToNeighborhood(const node_key_t& neigbourId, const cell_key_t& cellId);
  void addToNeighborhood(const node_key_t& neigbourId, const traci::TraCIPosition& pos);
  cell_key_t getCellId(const traci::TraCIPosition& pos) const;

 private:
  // Map node_key_t to cell_key_t to track if a node moved between cells.
  std::map<N, C> neighborhood;
};
#include "DcdMap.tcc"

}  // namespace crownet
