/*
 * RegularDcdMap.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularDcdMap.h"

#include "crownet/dcd/identifier/CellKeyProvider.h"

namespace crownet {

RegularDcdMap RegularDcdMapFactory::create(const IntIdentifer& ownerID) {
  std::shared_ptr<GridCellIDKeyProvider> provider =
      std::make_shared<GridCellIDKeyProvider>(gridSize, gridDim);
  return RegularDcdMap(ownerID, provider, std::make_shared<SimTimeProvider>());
}

std::shared_ptr<RegularDcdMap> RegularDcdMapFactory::create_shared_ptr(
    const IntIdentifer& ownerID) {
  std::shared_ptr<GridCellIDKeyProvider> provider =
      std::make_shared<GridCellIDKeyProvider>(gridSize, gridDim);
  return std::make_shared<RegularDcdMap>(ownerID, provider,
                                         std::make_shared<SimTimeProvider>());

std::shared_ptr<GridCellDistance> RegularDcdMapFactory::createDistanceProvider(){
    return std::make_shared<GridCellDistance>(gridDim);
}
}

}  // namespace crownet
