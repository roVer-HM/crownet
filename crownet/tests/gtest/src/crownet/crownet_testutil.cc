/*
 * crownet_util.cc
 *
 *  Created on: Apr 14, 2022
 *      Author: vm-sts
 */

#include "crownet_testutil.h"
#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"


using namespace crownet;


void expect_different_object_but_same_content(RegularCell::entry_t_ptr a, RegularCell::entry_t_ptr b){
    EXPECT_TRUE(a->operator ==(*b)) << "content of shared pointer should be identical";
    EXPECT_NE(a, b) << "expected different pointers";
}


void CellTest::init_grid(inet::Coord grid_offset, inet::Coord grid_bound, double cell_size){
    DcdFactoryProvider f = DcdFactoryProvider(
            grid_offset,
            grid_bound,
            cell_size);
    this->dcdFactory = f.dcdFactory;
    this->grid = f.grid;
    this->cell = RegularCell(dcdFactory->getTimeProvider(), GridCellID(5, 4), ownerId);
    this->cellEmpty = RegularCell(dcdFactory->getTimeProvider(), GridCellID(6, 5), ownerId);
}

void CellTest::add_entry(RegularCell::map_t& entryMap, IntIdentifer id, int count, simtime_t time, EntryDist dist, int rsd){
    auto e = ctor.entry();
    e->setSource(14);
    e->incrementCount(time, count);
    e->setEntryDist(dist);
    e->setResourceSharingDomainId(rsd);
    entryMap[id] = e;
}


void CellTest::setup_cells(){
    setSimTime(0.0);
    int increment_by = 1;

    auto& cell_data = cell.getData();
    // Cell with 5 values from 5 different sources, including own measurement.

    // Node 14 has a cell count of 2 with measuring distance of 3.0 m at time 3.0
    add_entry(cell.getData(), IntIdentifer(14), 2, SimTime(3.0), EntryDist{5.0, 3.0, 4.0}, ownerRsd);
    add_entry(cellNoLocal.getData(), IntIdentifer(14), 2, SimTime(3.0), EntryDist{5.0, 3.0, 4.0}, ownerRsd);

    // Node 15 has a cell count of 1 with measuring distance of 10.0 m at time 1.0
    add_entry(cell.getData(), IntIdentifer(15), 1, SimTime(1.0), EntryDist{5.0, 10.0, 4.0}, ownerRsd);
    add_entry(cellNoLocal.getData(), IntIdentifer(15), 1, SimTime(1.0), EntryDist{5.0, 10.0, 4.0}, ownerRsd);

    // Node 16 has a cell count of 2 with measuring distance of 2.0 m at time 1.0
    add_entry(cell.getData(), IntIdentifer(16), 2, SimTime(1.0), EntryDist{5.0, 2.0, 4.0}, ownerRsd);
    add_entry(cellNoLocal.getData(), IntIdentifer(16), 2, SimTime(1.0), EntryDist{5.0, 2.0, 4.0}, ownerRsd);

    // Node 17 has a cell count of 1 with measuring distance of 20.0 m at time 2.0
    add_entry(cell.getData(), IntIdentifer(17), 1, SimTime(2.0), EntryDist{5.0, 20.0, 4.0}, ownerRsd);
    add_entry(cellNoLocal.getData(), IntIdentifer(17), 1, SimTime(2.0), EntryDist{5.0, 20.0, 4.0}, ownerRsd);




    // Node 42 (cell owner) has cell count of 4 (with ids: 5, 9, 3, 19) with measuring distance 0.0m at time 1.0
    auto gentry = ctor.globalEntry();
    cell_data[IntIdentifer(42)] = gentry;  // count 4
    gentry->setSource(42);
    gentry->incrementCount(1, increment_by); // +1 (1)  // time, increment
    gentry->nodeIds.insert(5);
    gentry->incrementCount(1, increment_by); // +1 (2)  // time, increment
    gentry->nodeIds.insert(9);
    gentry->incrementCount(1, increment_by); // +1 (3)  // time, increment
    gentry->nodeIds.insert(3);
    gentry->incrementCount(1, increment_by); // +1 (4)  // time, increment
    gentry->nodeIds.insert(19);
    gentry->setEntryDist(EntryDist{}); // {0, 0, 0}  w=0.0 -> 1.0/1.0 (same cell)
    gentry->setResourceSharingDomainId(ownerRsd);
}









void MapTest::init_grid(inet::Coord grid_offset, inet::Coord grid_bound, double cell_size){
    DcdFactoryProvider f = DcdFactoryProvider(
            grid_offset,
            grid_bound,
            cell_size);
    this->dcdFactory = f.dcdFactory;
    this->grid = f.grid;
}


void MapTest::update(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, int count, omnetpp::simtime_t t){
    update(map, cell, id, count, t, EntryDist{0., 0., 0.});
}


void MapTest::update(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, int count, omnetpp::simtime_t t, double dist){
    update(map, cell, id, count, t, EntryDist{0., dist, 0.});
}

void MapTest::update(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, int count, omnetpp::simtime_t t, EntryDist distEntry) {
  auto _t = setSimTime(t);
  auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id), std::move(distEntry));
  map->setEntry(cell, std::move(e));
  setSimTime(_t);
}

void MapTest::incr(std::shared_ptr<RegularDcdMap> map, double x, double y, int id, double t) {
  auto _t = setSimTime(t);
  auto e = map->getEntry<GridGlobalEntry>(traci::TraCIPosition(x, y));
  e->incrementCount(t);
  e->nodeIds.insert(IntIdentifer(id));
  e->setResourceSharingDomainId(ownRessourceSharingDomainId);
  setSimTime(_t);
}

void MapTest::incr(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, double t) {
  auto _t = setSimTime(t);
  auto e = map->getEntry<GridGlobalEntry>(cell);
  e->incrementCount(t);
  e->nodeIds.insert(IntIdentifer(id));
  e->setResourceSharingDomainId(ownRessourceSharingDomainId);
  setSimTime(_t);
}

RegularCell::entry_t_ptr MapTest::entry(std::shared_ptr<RegularDcdMap> map, const SourceCellId& scid){
    return map->getEntry(scid.cell, scid.nodeId);
}



void MapTest::setup_local(std::shared_ptr<RegularDcdMap> mapLocal){
    auto _t = setSimTime(0.0);
    // [1, 1] count 2
    incr(mapLocal, 1.2, 1.2, 100, 30.0); // map, x, y, sourceId, time
    incr(mapLocal, 1.5, 1.5, 101, 30.0); // map, x, y, sourceId, time
    // [3, 3] count 3
    incr(mapLocal, 3.2, 3.2, 200, 32.0); // map, x, y, sourceId, time
    incr(mapLocal, 3.5, 3.5, 201, 32.0); // map, x, y, sourceId, time
    incr(mapLocal, 3.5, 3.5, 202, 32.0); // map, x, y, sourceId, time
    // [4, 4] count 1
    incr(mapLocal, 4.2, 4.5, 300, 34.0); // map, x, y, sourceId, time
    // local neighbors [ 100, 101, 200, 201, 202, 300] #6
    setSimTime(_t);
}

/**
 * [1, 1] Local = 2 / Other = 4
 * [3, 3] Local = 3 / Other = 3
 * [4, 4] Local = 2 / Other = 0
 * [6, 3] Local = 0 / Other = [5, 7, 9]
 */
void MapTest::setup_full(std::shared_ptr<RegularDcdMap> mapFull){
    auto _t = setSimTime(0.0);

    // [1, 1] Local = 2 / Other = 4
    incr(mapFull, 1.2, 1.2, 100, 30.0); // map, x, y, sourceId, time
    incr(mapFull, 1.5, 1.5, 101, 30.0); // map, x, y, sourceId, time
    update(mapFull, c1_1_800.cell, c1_1_800.nodeId, 4, 31.0); // map, x, y, sourceId, count, time
    // [3, 3] Local = 3 / Other = 3
    incr(mapFull, 3.2, 3.2, 200, 32.0);
    incr(mapFull, 3.5, 3.5, 201, 32.0);
    incr(mapFull, 3.5, 3.5, 202, 32.0);
    update(mapFull, c4_4_801.cell, c4_4_801.nodeId, 3, 32.0);
    // [4, 4] Local = 2 / Other = 0
    incr(mapFull, 4.2, 4.5, 300, 34.0);
    incr(mapFull, 4.2, 4.5, 301, 34.0);
    // [6, 3] Local =  0 / Other = [5, 7, 9]
    update(mapFull, c6_3_803.cell, c6_3_803.nodeId, 5, 33.0);
    update(mapFull, c6_3_805.cell, c6_3_805.nodeId, 7, 31.0);
    update(mapFull, c6_3_808.cell, c6_3_808.nodeId, 9, 12.0);

    setSimTime(_t);
}

void MapTest::setup_empty(std::shared_ptr<RegularDcdMap> mapEmpty){
    /* nothing to do */
}

void MapTest::clear_map(std::shared_ptr<RegularDcdMap> map){
    ClearVisitor v{simTime()};
    map->visitCells(v);
}

void MapTest::clear_map(std::shared_ptr<RegularDcdMap> map, simtime_t t){
    setSimTime(t);
    clear_map(map);
}

