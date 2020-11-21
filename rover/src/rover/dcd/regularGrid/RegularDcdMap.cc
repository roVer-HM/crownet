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
  return RegularDcdMap(ownerID, provider);
}

}  // namespace rover
