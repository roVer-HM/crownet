/*
 * RegularDcdMapPrinter.cc
 *
 *  Created on: Dec 2, 2020
 *      Author: sts
 */

#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"

namespace crownet {

void RegularDcdMapSqlValuePrinter::writeSqlStatement(std::shared_ptr<SqlLiteApi> sqlApi){

    for(const auto& val : map->valid()){
        const auto lEntry = val.second.val();
        int own_cell = (val.first == map->getOwnerCell()) ? 1 : 0;
        int owner_rsd_id = map->getResourceSharingDomainId();

        sqlApi->write_map(
                map->getOwnerId(), //hostId,
                omnetpp::simTime().dbl(), //simtime,
                val.first.x(), // x,
                val.first.y(), //y,
                lEntry->getCount(), // count,
                lEntry->getMeasureTime().dbl(), // measured_t,
                lEntry->getReceivedTime().dbl(), // received_t,
                lEntry->getSource(), // source,
                lEntry->getSelectedIn(), // selection,
                lEntry->getSelectionRank(), //selectionRank,
                lEntry->getEntryDist().sourceHost, // sourceHost,
                lEntry->getEntryDist().sourceEntry, // sourceEntry,
                lEntry->getEntryDist().hostEntry, // hostEntry,
                lEntry->getResourceSharingDomainId(), // rsd_id,
                own_cell, // own_cell,
                owner_rsd_id); //owner_rsd_id
    }
}

void RegularDcdMapSqlGlobalPrinter::writeSqlStatement(std::shared_ptr<SqlLiteApi> sqlApi){
    for(const auto& val : map->validLocal()){
        const auto lEntry = val.second.get<GridGlobalEntry>();

        int64_t glb_id = sqlApi->write_map_glb(
                omnetpp::simTime().dbl(), // simtime,
                val.first.x(), // x,
                val.first.y(), //y,
                lEntry->getCount()); // count,
        for (const auto& node_id : lEntry->nodeIds){
            sqlApi->write_glb_node_id_mapping( glb_id, node_id);
        }
  }
}


int RegularDcdMapValuePrinter::columns() const { return 9; }

void RegularDcdMapValuePrinter::writeTo(std::ostream& out,
                                        const std::string& sep) const {
  // for all cells in dcd map
    for (const auto& val : map->valid()) {
    int ownCell = (val.first == map->getOwnerCell()) ? 1 : 0;
    int owner_rsd_id = map->getResourceSharingDomainId();

    out << omnetpp::simTime().dbl() << sep;
    val.first.writeTo(out, sep);  // GridCellID (x, y)
    out << sep;
    val.second.val()->writeTo(out, sep);  // Entry (more than one column!)
    out << sep;
    out << ownCell << sep;
    out << owner_rsd_id << std::endl;

  }
}

void RegularDcdMapValuePrinter::writeHeaderTo(std::ostream& out,
                                              const std::string& sep) const {
  out << "simtime" << sep << \
          "x" << sep << \
          "y" << sep << \
          "count" << sep << \
          "measured_t" << sep << \
          "received_t" << sep << \
          "source" << sep << \
          "selection" << sep << \
          "selectionRank" << sep << \
          "sourceHost" << sep << \
          "sourceEntry" << sep << \
          "hostEntry" << sep << \
          "rsd_id" << sep << \
          "own_cell" << sep << \
          "owner_rsd_id" << std::endl;
}

void RegularDcdMapAllPrinter::writeTo(std::ostream& out,
                                      const std::string& sep) const {
  // for all cells in dcd map
    for (const auto& val : map->valid()){
    int ownCell = (val.first == map->getOwnerCell()) ? 1 : 0;
    int owner_rsd_id = map->getResourceSharingDomainId();
    // for all measurements in cell
    bool foundSelected = false;
    for (const auto& cell: val.second.validIter()){
      auto entry = cell.second;
      if (!foundSelected){
          foundSelected = !entry->getSelectedIn().empty();
      }

      out << omnetpp::simTime().dbl() << sep;
      val.first.writeTo(out, sep);  // GridCellID (x, y)
      out << sep;
      entry->writeTo(out, sep);  // Entry (more than one column!)
      out << sep;
      out << ownCell << sep;
      out << owner_rsd_id << std::endl;

    }
    if (!foundSelected){
        // selected value not found -> selected value was calculated.
        // add selected value manually
        out << omnetpp::simTime().dbl() << sep;
        val.first.writeTo(out, sep);  // GridCellID (x, y)
        out << sep;
        val.second.val()->writeTo(out, sep);  // Entry
        out << sep;
        out << ownCell << sep;
        out << owner_rsd_id << std::endl;
    }
  }
}

void RegularDcdMapGlobalPrinter::writeTo(std::ostream& out,
                                         const std::string& sep) const {
    for(const auto& val : map->validLocal()){
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
  out << "simtime" << sep << \
          "x" << sep << \
          "y" << sep << \
          "count" << sep  << \
          "measured_t" << sep << \
          "received_t" << sep << \
          "source" << sep << \
          "selection" << sep << \
          "selectionRank" << sep << \
          "own_cell" << sep << \
          "node_id" << std::endl;
}

}  // namespace crownet
