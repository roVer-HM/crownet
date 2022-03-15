/*
 * CellKeyProvider.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include "inet/common/geometry/Geometry_m.h"
#include "crownet/dcd/generic/Cell.h"
#include "traci/Position.h"
#include "crownet/common/RegularGridInfo.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"

namespace crownet {

template <typename C>
class CellKeyProvider {
 public:
  using CellPair = std::pair<const C, const C>;

  virtual ~CellKeyProvider() = default;

  virtual C getCellKey(const traci::TraCIPosition& pos) = 0;
  virtual traci::TraCIPosition getCellPosition(const traci::TraCIPosition& pos) = 0;
  virtual traci::TraCIPosition getCellPosition(const GridCellID& gridCell) = 0;

  virtual inet::Coord  getCellSize() const = 0;
  virtual inet::Coord  getGridSize() const = 0;
  virtual double cellCenterDist(const C& cell1, const C&  cell2) = 0;
  virtual double maxCellDist(const C& cell1, const C&  cell2) const = 0;
  virtual int maxIdCellDist(const C& cell1, const C&  cell2) const = 0;

  virtual EntryDist getEntryDist(const C& source, const C& owner, const C& entry)=0;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const C& entry)=0;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry, const double sourceEntry) = 0;

};



class GridCellIDKeyProvider : public CellKeyProvider<GridCellID> {
 public:
  GridCellIDKeyProvider(const RegularGridInfo& gridInfo)
      : gridInfo(gridInfo) {}
  GridCellIDKeyProvider(std::shared_ptr<OsgCoordinateConverter> converter)
      : converter(converter), gridInfo(converter->getGridDescription()) {}

  virtual GridCellID getCellKey(const traci::TraCIPosition& pos) override;
  virtual traci::TraCIPosition getCellPosition(const traci::TraCIPosition& pos) override;
  virtual traci::TraCIPosition getCellPosition(const GridCellID& gridCell) override;

  virtual inet::Coord  getCellSize() const override { return converter->getCellSize(); }
  virtual inet::Coord  getGridSize() const override { return converter->getGridSize(); }

  virtual double cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2) override;
  virtual double maxCellDist(const GridCellID& cell1, const GridCellID&  cell2) const override
      { return gridInfo.maxCellDist(cell1, cell2);}
  virtual int maxIdCellDist(const GridCellID& cell1, const GridCellID&  cell2) const override
      { return gridInfo.maxIdCellDist(cell1, cell2);}

  virtual EntryDist getEntryDist(const GridCellID& source, const GridCellID& owner, const GridCellID& entry) override;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry) override;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry, const double sourceEntry) override;

 private:
  std::shared_ptr<OsgCoordinateConverter> converter;
  RegularGridInfo gridInfo;
  std::map<const CellPair, double> distMap;

};

}  // namespace crownet
