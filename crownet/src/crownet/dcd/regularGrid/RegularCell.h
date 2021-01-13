/*
 * RegularCell.h
 *
 *  Created on: Nov 23, 2020
 *      Author: sts
 */

#pragma once

#include <omnetpp/simtime.h>

#include "crownet/dcd/generic/Cell.h"

namespace crownet {
///////////////////////////////////////////////////////////////////////////////
/// Default Cell specializations   ////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// Cell, Node, Time
using RegularCell = Cell<GridCellID, IntIdentifer, omnetpp::simtime_t>;

std::ostream& operator<<(std::ostream& os, const RegularCell::value_type& i);

}  // namespace crownet
