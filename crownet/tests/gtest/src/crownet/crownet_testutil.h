/*
 * crownet_util.h
 *
 *  Created on: Apr 14, 2022
 *      Author: vm-sts
 */

#ifndef GTEST_SRC_CROWNET_CROWNET_TESTUTIL_H_
#define GTEST_SRC_CROWNET_CROWNET_TESTUTIL_H_

#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "main_test.h"

using namespace crownet;


void expect_different_object_but_same_content(RegularCell::entry_t_ptr a, RegularCell::entry_t_ptr b);

class DcdFactoryProvider {
public:
    ~DcdFactoryProvider()=default;
    DcdFactoryProvider() : DcdFactoryProvider(inet::Coord(.0, .0), inet::Coord(10.0, 10.0),1.0){}
    DcdFactoryProvider(inet::Coord offset, inet::Coord bound, double cellSize)
        : DcdFactoryProvider(offset, bound, inet::Coord(cellSize, cellSize)) {}
    DcdFactoryProvider(inet::Coord offset, inet::Coord bound, inet::Coord cellSize){
        converter = std::make_shared<OsgCoordinateConverter>(
                offset,
                bound,
                "EPSG:32632");
        converter->setCellSize(cellSize);
        grid = converter->getGridDescription();
        dcdFactory = std::make_shared<RegularDcdMapFactory>(converter);
    }

    std::shared_ptr<OsgCoordinateConverter> converter;
    RegularGridInfo grid;
    std::shared_ptr<RegularDcdMapFactory> dcdFactory;
};


class CellTest : public BaseOppTest {
public:
    CellTest(): BaseOppTest(), ownerId(IntIdentifer(42)), ownerRsd(1){
        init_grid();
    }

    virtual void init_grid(inet::Coord grid_offset = inet::Coord(0., 0.), inet::Coord grid_bound = inet::Coord(10.0, 10.0), double cell_size = 1.0);

    virtual void add_entry(RegularCell::map_t& entryMap, IntIdentifer id, int count, simtime_t time, EntryDist dist, int rsd);

    virtual void setup_cells();

protected:
    std::shared_ptr<RegularDcdMapFactory> dcdFactory;
    RegularGridInfo  grid;
    RegularCell cell;
    RegularCell cellNoLocal;
    RegularCell cellEmpty;
    RegularCell::entry_ctor_t ctor;

    IntIdentifer ownerId;
    int ownerRsd;

};


struct SourceCellId{
    GridCellID cell;
    IntIdentifer nodeId;

    SourceCellId(int x, int y, int id): cell(GridCellID{x, y}), nodeId(IntIdentifer{id}){}
};


class MapTest : public BaseOppTest {
public:
    MapTest(): BaseOppTest(){init_grid();}
    using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;


    virtual void init_grid(inet::Coord grid_offset = inet::Coord(0., 0.), inet::Coord grid_bound = inet::Coord(10.0, 10.0), double cell_size = 1.0);


    void update(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, int count, omnetpp::simtime_t t);
    void update(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, int count, omnetpp::simtime_t t, double dist);
    void update(std::shared_ptr<RegularDcdMap> map, GridCellID cell, int id, int count, omnetpp::simtime_t t, EntryDist distEntry);

    void incr(std::shared_ptr<RegularDcdMap> map, double x, double y, int i, double t);


    void setup_local(std::shared_ptr<RegularDcdMap> mapLocal);
    void setup_full(std::shared_ptr<RegularDcdMap> mapFull);
    void setup_empty(std::shared_ptr<RegularDcdMap> mapEmpty);
    void clear_map(std::shared_ptr<RegularDcdMap> map);
    void clear_map(std::shared_ptr<RegularDcdMap> map, simtime_t t);

    RegularCell::entry_t_ptr entry(std::shared_ptr<RegularDcdMap>, const SourceCellId& scid );

protected:
    std::shared_ptr<RegularDcdMapFactory> dcdFactory;
    RegularGridInfo  grid;
    int ownRessourceSharingDomainId = 5;
    int otherRessourceSharingDomainId = 500;

    // combination off cell id and node id used for foreign cell data.
    SourceCellId c1_1_800{1,1,800};
    SourceCellId c4_4_801{4,4,801};
    SourceCellId c6_3_803{6,3,803};
    SourceCellId c6_3_805{6,3,805};
    SourceCellId c6_3_808{6,3,808};
};


#endif /* GTEST_SRC_CROWNET_CROWNET_TESTUTIL_H_ */
