/*
 * CellKeyProvider.h
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#pragma once

#include "inet/common/geometry/Geometry_m.h"
#include "crownet/dcd/generic/Cell.h"
#include "crownet/crownet.h"
#include "traci/Position.h"
#include "crownet/common/RegularGridInfo.h"
#include "crownet/common/converter/OsgCoordinateConverter.h"

namespace crownet {

template <typename C>
class CellKeyProvider {
 public:
  using CellPair = std::pair<const C, const C>;

  virtual ~CellKeyProvider() = default;

  virtual const C getCellKey(const traci::TraCIPosition& pos) const = 0 ;
  virtual const C getCellKey(const inet::Coord& pos) const = 0;
  virtual const C getCellKey(const int cellKey1D) const = 0;

  virtual const int getCellKey1D(const traci::TraCIPosition& pos) const = 0;
  virtual const int getCellKey1D(const inet::Coord& pos) const = 0;
  virtual const int getCellKey1D(const C& cellKey) const =0;
  virtual bool changedCell(const traci::TraCIPosition& pos1, const traci::TraCIPosition& pos2) = 0;
  virtual bool changedCell(const inet::Coord& pos1, const inet::Coord& pos2) = 0;
  virtual const traci::TraCIPosition getCellPosition(const traci::TraCIPosition& pos) = 0;
  virtual const traci::TraCIPosition getCellPosition(const GridCellID& gridCell) = 0;

  virtual inet::Coord  getCellSize() const = 0;
  virtual inet::Coord  getGridSize() const = 0;
  virtual const inet::Coord  cellCenter(const traci::TraCIPosition& pos) const = 0;
  virtual const inet::Coord cellCenter(const inet::Coord& pos) const = 0;
  virtual const inet::Coord cellCenter(const C& cellKey) const = 0;
  virtual const inet::Coord cellCenter(const int cellKey1D) const = 0;

  virtual double cellCenterDist(const C& cell1, const C&  cell2) = 0;
  virtual double maxCellDist(const C& cell1, const C&  cell2) const = 0;

  virtual EntryDist getEntryDist(const C& source, const C& owner, const C& entry)=0;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const C& entry)=0;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry, const double sourceEntry) = 0;

  virtual bool cellInRadius(const GridCellID& cell, const double radius, const traci::TraCIPosition& circleCenter) const = 0;

  virtual bool intersectLineCircle(const inet::Coord& a, const inet::Coord& b, const double radiusSquared, const inet::Coord& center) const  = 0;
  virtual std::vector<C> getCellsInRadius(const inet::Coord& pos, double distance) const = 0;
//  virtual std::vector<C> getCellsInRadius(const traci::TraCIPosition& pos, double distance) const = 0;

};



class GridCellIDKeyProvider : public CellKeyProvider<GridCellID> {
 public:
//  GridCellIDKeyProvider(const RegularGridInfo& gridInfo)
//      : gridInfo(gridInfo) {}
  GridCellIDKeyProvider(std::shared_ptr<OsgCoordinateConverter> converter)
      : converter(converter), gridInfo(converter->getGridDescription()) {}

  virtual const GridCellID getCellKey(const traci::TraCIPosition& pos) const override;
  virtual const GridCellID getCellKey(const inet::Coord& pos) const override;
  virtual const GridCellID getCellKey(const int cellKey1D) const override;

  virtual const int getCellKey1D(const traci::TraCIPosition& pos) const override;
  virtual const int getCellKey1D(const inet::Coord& pos) const override;
  virtual const int getCellKey1D(const GridCellID& cellKey) const override;

  virtual bool changedCell(const traci::TraCIPosition& pos1, const traci::TraCIPosition& pos2) override;
  virtual bool changedCell(const inet::Coord& pos1, const inet::Coord& pos2) override;

  virtual const traci::TraCIPosition getCellPosition(const traci::TraCIPosition& pos) override;
  virtual const traci::TraCIPosition getCellPosition(const GridCellID& gridCell) override;

  virtual inet::Coord  getCellSize() const override { return converter->getCellSize(); }
  virtual inet::Coord  getGridSize() const override { return converter->getGridSize(); }

  virtual const inet::Coord cellCenter(const traci::TraCIPosition& pos) const override;
  virtual const inet::Coord cellCenter(const inet::Coord& pos) const override;
  virtual const inet::Coord cellCenter(const GridCellID& cellKey) const override;
  virtual const inet::Coord cellCenter(const int cellKey1D) const override;


  virtual double cellCenterDist(const GridCellID& cell1, const GridCellID&  cell2) override;
  virtual double maxCellDist(const GridCellID& cell1, const GridCellID&  cell2) const override
      { return gridInfo.maxCellDist(cell1, cell2);}

  virtual EntryDist getEntryDist(const GridCellID& source, const GridCellID& owner, const GridCellID& entry) override;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry) override;
  virtual EntryDist getExactDist(const inet::Coord source, const inet::Coord owner, const GridCellID& entry, const double sourceEntry) override;

  virtual bool cellInRadius(const GridCellID& cell, const double radius, const traci::TraCIPosition& circleCenter) const override;

  virtual bool intersectLineCircle(const inet::Coord& a, const inet::Coord& b, const double radiusSquared, const inet::Coord& center) const  override;
  virtual std::vector<GridCellID> getCellsInRadius(const inet::Coord& pos, double distance) const override;
//  virtual std::vector<GridCellID> getCellsInRadius(const traci::TraCIPosition& pos, double distance) const override;


 private:
  std::shared_ptr<OsgCoordinateConverter> converter;
  RegularGridInfo gridInfo;
  std::map<const CellPair, double> distMap;

};

}  // namespace crownet
