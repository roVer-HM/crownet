/*
 * RegularDcdMap.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "rover/dcd/regularGrid/RegularDcdMap.h"

#include "rover/dcd/identifier/CellKeyProvider.h"

namespace rover {

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
}

}  // namespace rover
