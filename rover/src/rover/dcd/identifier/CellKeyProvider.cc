/*
 * CellKeyProvider.cc
 *
 *  Created on: Nov 30, 2020
 *      Author: sts
 */

#include "rover/dcd/identifier/CellKeyProvider.h"

#include <math.h>

namespace rover {

GridCellID GridCellIDKeyProvider::getCellKey(const traci::TraCIPosition& pos) {
  int x_id = floor(pos.x / gridSize.first);
  int y_id = floor(pos.y / gridSize.second);
  return GridCellID(x_id, y_id);
}

}  // namespace rover
