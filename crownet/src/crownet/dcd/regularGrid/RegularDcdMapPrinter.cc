/*
 * RegularDcdMapPrinter.cc
 *
 *  Created on: Dec 2, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"

namespace crownet {

int RegularDcdMapValuePrinter::columns() const { return 9; }

void RegularDcdMapValuePrinter::writeTo(std::ostream& out,
                                        const std::string& sep) const {
  // for all cells in dcd map
  for (auto it = map->valid(); it != it.end(); ++it) {
    auto val = *it;
    int ownCell = (val.first == map->getOwnerCell()) ? 1 : 0;

    out << omnetpp::simTime().dbl() << sep;
    val.first.writeTo(out, sep);  // GridCellID (x, y)
    out << sep;
    val.second.val()->writeTo(out, sep);  // Entry
    out << sep;
    out << ownCell << std::endl;
  }
}

void RegularDcdMapValuePrinter::writeHeaderTo(std::ostream& out,
                                              const std::string& sep) const {
  out << "simtime" << sep << "x" << sep << "y" << sep << "count" << sep
      << "measured_t" << sep << "received_t" << sep << "source" << sep
      << "selection" << sep << "own_cell" << std::endl;
}

void RegularDcdMapAllPrinter::writeTo(std::ostream& out,
                                      const std::string& sep) const {
  // for all cells in dcd map
  for (auto it = map->valid(); it != it.end(); ++it) {
    auto val = *it; // [cell_id, cell]
    int ownCell = (val.first == map->getOwnerCell()) ? 1 : 0;
    // for all measurements in cell
    bool foundSelected = false;
    for (auto cIt = val.second.validIter(); cIt != cIt.end(); ++cIt) {
      auto entry = (*cIt).second;
      if (!foundSelected){
          foundSelected = !entry->getSelectedIn().empty();
      }

      out << omnetpp::simTime().dbl() << sep;
      val.first.writeTo(out, sep);  // GridCellID (x, y)
      out << sep;
      entry->writeTo(out, sep);  // Entry
      out << sep;
      out << ownCell << std::endl;
    }
    if (!foundSelected){
        // selected value not found -> selected value was calculated.
        // add selected value manually
        out << omnetpp::simTime().dbl() << sep;
        val.first.writeTo(out, sep);  // GridCellID (x, y)
        out << sep;
        val.second.val()->writeTo(out, sep);  // Entry
        out << sep;
        out << ownCell << std::endl;
    }
  }
}

void RegularDcdMapGlobalPrinter::writeTo(std::ostream& out,
                                         const std::string& sep) const {
  for (auto it = map->validLocal(); it != it.end(); ++it) {
    auto val = *it;
    int ownCell = (val.first == map->getOwnerCell()) ? 1 : 0;
    const auto lEntry = val.second.get<GridGlobalEntry>();

    out << omnetpp::simTime().dbl() << sep;
    val.first.writeTo(out, sep);  // GridCellID (x, y)
    out << sep;
    lEntry->writeTo(out, sep);  // Entry
    out << sep;
    out << ownCell;
    out << sep << lEntry->nodeString(sep) << std::endl;
  }
}

void RegularDcdMapGlobalPrinter::writeHeaderTo(std::ostream& out,
                                               const std::string& sep) const {
  out << "simtime" << sep << "x" << sep << "y" << sep << "count" << sep
      << "measured_t" << sep << "received_t" << sep << "source" << sep
      << "selection" << sep << "own_cell" << sep << "node_id" << std::endl;
}

}  // namespace crownet
