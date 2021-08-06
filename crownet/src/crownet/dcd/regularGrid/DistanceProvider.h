/*
 * DistanceProvider.h
 *
 *  Created on: Jul 29, 2021
 *      Author: vm-sts
 */

#pragma once

#include "crownet/common/Entry.h"
#include "crownet/dcd/identifier/Identifiers.h"
#include <map>

namespace crownet {


/**
 * return eukleadian distance and cache distance between cells.
 */
class GridCellDistance {
public:
    using CellPair = std::pair<const GridCellID, const GridCellID>;
    virtual ~GridCellDistance() = default;
    GridCellDistance(const std::pair<double, double>& gridDim);
    GridCellDistance(const double xDim, const double yDim);
    double dist(const GridCellID& a, const GridCellID& b);
    EntryDist getEntryDist(const GridCellID& source, const GridCellID& owner, const GridCellID& entry);

private:
    double xDim;
    double yDim;
    std::map<const CellPair, double> distMap;

};

} /* namespace crownet */

