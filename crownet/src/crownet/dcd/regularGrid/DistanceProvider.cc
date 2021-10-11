/*
 * DistanceProvider.cc
 *
 *  Created on: Jul 29, 2021
 *      Author: vm-sts
 */

#include "DistanceProvider.h"
#include <math.h>

namespace crownet {

GridCellDistance::GridCellDistance(const double xDim, const double yDim)
    : xDim(xDim), yDim(yDim){}

double GridCellDistance::dist(const GridCellID& a, const GridCellID& b){
    // only cache upper triangular matrix with (a<b) and a!=b
    if (a == b){
       return 0.0;
    } else if (b < a){
        // Recursive call to ensure a < b
        return this->dist(b, a);
    }
    // a < b after here.
    const auto p = std::make_pair(a, b);
    if (distMap.find(p) == distMap.end()){
        double dist = std::sqrt(
                std::pow(xDim * (b.x() - a.x()), 2) +
                std::pow(yDim * (b.y() - a.y()), 2)
                );
        distMap[p] = dist;
        return dist;

    } else {
        return distMap[p]; // return cache
    }
}

EntryDist GridCellDistance::getEntryDist(const GridCellID& source,
        const GridCellID& owner, const GridCellID& entry){
    EntryDist entryDist;
    entryDist.sourceEntry = dist(source, entry);
    entryDist.sourceOwner = dist(source, owner);
    entryDist.ownerEntry = dist(owner, entry);
    return entryDist;
}

} /* namespace crownet */
