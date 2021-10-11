/*
 * CellKeyProvider.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "crownet/dcd/identifier/CellKeyProvider.h"

#include <math.h>

namespace crownet {

GridCellID GridCellIDKeyProvider::getCellKey(const traci::TraCIPosition& pos) {
  int x_id = floor(pos.x / cellSize.first);
  int y_id = floor(pos.y / cellSize.second);
  return GridCellID(x_id, y_id);
}

}  // namespace crownet
