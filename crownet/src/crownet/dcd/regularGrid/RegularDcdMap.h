/*
 * Regular_dcdMap.h
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#pragma once

#include "crownet/dcd/generic/Cell.h"
#include "crownet/dcd/generic/DcdMap.h"
#include "crownet/dcd/generic/DcdMapWatcher.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/generic/CellIdStream.h"
#include "crownet/common/RegularGridInfo.h"
#include "crownet/applications/dmap/dmap_m.h"



#include <functional>

namespace crownet {

using RegularDcdMap = DcDMap<GridCellID, IntIdentifer, omnetpp::simtime_t>;
using RegularDcdMapWatcher = DcdMapWatcher<GridCellID, IntIdentifer, omnetpp::simtime_t>;
using VisitorCreator = std::function<std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>>(MapCfg*)>;
using CellIdStreamCreator = std::function<std::shared_ptr<ICellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>>()>;
using GridEntry = IEntry<IntIdentifer, omnetpp::simtime_t>;
using GridGlobalEntry = IGlobalEntry<IntIdentifer, omnetpp::simtime_t>;

class RegularDcdMapFactory {
 public:
//  RegularDcdMapFactory(const RegularGridInfo& grid);
  RegularDcdMapFactory(std::shared_ptr<OsgCoordinateConverter> converter);

  std::map<std::string, VisitorCreator>visitor_dispatcher;
  std::map<std::string, CellIdStreamCreator>cellIdStream_dispatcher;

  RegularDcdMap create(const IntIdentifer& ownerID, const std::string& idStreamType = "default");
  std::shared_ptr<RegularDcdMap> create_shared_ptr(const IntIdentifer& ownerID, const std::string& idStreamType = "default");
  std::shared_ptr<TimestampedGetEntryVisitor<RegularCell>> createValueVisitor(MapCfg* mapCfg);
  std::shared_ptr<ICellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t>> createCellIdStream(const std::string& typeName);
  std::shared_ptr<GridCellIDKeyProvider> getCellKeyProvider() { return cellKeyProvider; }
  RegularGridInfo getGrid() const {return grid;}
  std::shared_ptr<SimTimeProvider> getTimeProvider() { return timeProvider; }

 private:
  RegularGridInfo grid;
  std::shared_ptr<OsgCoordinateConverter> converter;
  std::shared_ptr<GridCellIDKeyProvider> cellKeyProvider;
  std::shared_ptr<SimTimeProvider> timeProvider;
};

}  // namespace crownet
