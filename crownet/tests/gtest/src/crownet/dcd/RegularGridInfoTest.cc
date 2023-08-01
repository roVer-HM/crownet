/*
 * RegularGridInfoTest.cc
 *
 *  Created on: Sep 28, 2022
 *      Author: vm-sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"
#include "crownet/crownet.h"
#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "inet/common/geometry/common/Coord.h"

using namespace crownet;

bool operator== (const libsumo::TraCIPosition&  p1, const libsumo::TraCIPosition&  p2){
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

bool cmp (const libsumo::TraCIPosition&  p1, const libsumo::TraCIPosition&  p2){
    return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

class RegularGridInfoTest_F : public BaseOppTest {
public:
    RegularGridInfoTest_F(){
        DcdFactoryProvider provider_s = DcdFactoryProvider(
                inet::Coord(0.0, 0.0),
                inet::Coord(100.0, 100.0),
                inet::Coord(10.0, 10.0)
        );
        s = provider_s.converter;
        aoi_s.setX(0.);
        aoi_s.setY(0.);
        aoi_s.setWidth(22.);
        aoi_s.setHeight(22.);
        //aoi should be?
        s->setAreaOfInterest(&aoi_s);
        ss = s->getGridDescription();

        DcdFactoryProvider provider_r = DcdFactoryProvider(
                inet::Coord(0.0, 0.0),
                inet::Coord(100.0, 200.0),
                inet::Coord(10.0, 20.0)
        );
        r = provider_r.converter;
        aoi_r.setX(10.);
        aoi_r.setY(20.);
        aoi_r.setWidth(22.);
        aoi_r.setHeight(22.);
        r->setAreaOfInterest(&aoi_r);
        rr = r->getGridDescription();

    }

    protected:
    std::shared_ptr<OsgCoordinateConverter> s;
    RegularGridInfo ss;
    AreaOfInterest aoi_s;
    std::shared_ptr<OsgCoordinateConverter> r;
    RegularGridInfo rr;
    AreaOfInterest aoi_r;
};

TEST_F(RegularGridInfoTest_F, inCellCenter){
    auto grid = s->getGridDescription();
    // contains lower and left edge but not right and upper
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(0.0, 2.0)));
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(2.0, 0.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(10.0, 2.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(2.0, 10.0)));
    // contains center
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(5.0, 5.0)));

    grid = r->getGridDescription();
    // contains lower and left edge but not right and upper
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 10.0), inet::Coord(0.0, 15.0)));
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(7.0, 0.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(7.0, 20.0)));
    EXPECT_FALSE(grid.posInCenteredCell(inet::Coord(5.0, 5.0), inet::Coord(10.0, 15.0)));
    // contains center
    EXPECT_TRUE(grid.posInCenteredCell(inet::Coord(5.0, 10.0), inet::Coord(5.0, 10.0)));
}


TEST_F(RegularGridInfoTest_F, getGridSize){
    EXPECT_EQ(ss.getGridSize(), inet::Coord(100., 100.));
    EXPECT_EQ(rr.getGridSize(), inet::Coord(100., 200.));
}

TEST_F(RegularGridInfoTest_F, getBound){
    EXPECT_EQ(ss.getBound(), inet::Coord(100., 100.));
    EXPECT_EQ(rr.getBound(), inet::Coord(100., 200.));
}

TEST_F(RegularGridInfoTest_F, getCellSize){
    EXPECT_EQ(ss.getCellSize(), inet::Coord(10., 10.));
    EXPECT_EQ(rr.getCellSize(), inet::Coord(10., 20.));
}

TEST_F(RegularGridInfoTest_F, getCellCount){
    EXPECT_EQ(ss.getCellCount(), inet::Coord(10., 10.));
    EXPECT_EQ(rr.getCellCount(), inet::Coord(10., 10.));
}

TEST_F(RegularGridInfoTest_F, getAreaOfIntrest){
    auto _aoi_s = ss.getAreaOfIntrest();
    EXPECT_TRUE(_aoi_s.lowerLeftPosition() == libsumo::TraCIPosition(0.0, 0.0));
    EXPECT_TRUE(_aoi_s.upperRightPosition() == libsumo::TraCIPosition(22.0, 22.0));

    auto _aoi_r = rr.getAreaOfIntrest();
    EXPECT_TRUE(_aoi_r.lowerLeftPosition() == libsumo::TraCIPosition(10.0, 20.0));
    EXPECT_TRUE(_aoi_r.upperRightPosition() == libsumo::TraCIPosition(32.0, 42.0));
}


TEST_F(RegularGridInfoTest_F, aoiIter){
    auto iter_s = ss.aoiIter();
    int i = 0;
    std::vector<GridCellID> ids = {GridCellID(0,0), GridCellID(1, 0), GridCellID(0,1), GridCellID(1,1)};
    for(const auto& x : iter_s){
        EXPECT_EQ(x, ids[i]);
        i++;
    }
    i = 0;
    std::vector<GridCellID> ids2 = {GridCellID(1,1), GridCellID(2,1), GridCellID(3,1),
                                    GridCellID(1,2), GridCellID(2,2), GridCellID(3,2)};
    for(const auto& x : rr.aoiIter()){
        EXPECT_EQ(x, ids2[i]);
        i++;
    }
}


TEST_F(RegularGridInfoTest_F, getCellCenter1){
    EXPECT_TRUE(cmp(ss.getCellCenter(GridCellID(1,1)), traci::TraCIPosition(15., 15.)));
    EXPECT_TRUE(cmp(rr.getCellCenter(GridCellID(1,1)), traci::TraCIPosition(15., 30.)));
}

TEST_F(RegularGridInfoTest_F, getCellCenter2){
    EXPECT_TRUE(cmp(ss.getCellCenter(1,1), traci::TraCIPosition(15., 15.)));
    EXPECT_TRUE(cmp(rr.getCellCenter(1,1), traci::TraCIPosition(15., 30.)));
}

TEST_F(RegularGridInfoTest_F, getCellCenter3){
    EXPECT_TRUE(cmp(ss.getCellCenter(1,1), traci::TraCIPosition(15., 15.)));
    EXPECT_TRUE(cmp(rr.getCellCenter(1,1), traci::TraCIPosition(15., 30.)));
}

TEST_F(RegularGridInfoTest_F, getCellKey){
    EXPECT_EQ(ss.getCellKey(traci::TraCIPosition(52., 53.)), GridCellID(5,5));
    EXPECT_EQ(rr.getCellKey(traci::TraCIPosition(52., 53.)), GridCellID(5,2));
}

TEST_F(RegularGridInfoTest_F, getCellId1){
    EXPECT_EQ(ss.getCellKey(12), GridCellID(2,1));
    EXPECT_EQ(ss.getCellKey(-12), GridCellID(2,1));
    EXPECT_EQ(rr.getCellKey(12), GridCellID(2,1));
    EXPECT_EQ(rr.getCellKey(-12), GridCellID(2,1));
}

TEST_F(RegularGridInfoTest_F, getCellId2){
    EXPECT_EQ(ss.getCellKey1D(2, 1), 12);
    EXPECT_EQ(rr.getCellKey1D(2, 1), 12);
}

TEST_F(RegularGridInfoTest_F, posInCenteredCell){
    EXPECT_TRUE(ss.posInCenteredCell(inet::Coord(15., 15.), inet::Coord(17., 11.)));
    EXPECT_TRUE(ss.posInCenteredCell(inet::Coord(15., 15.), inet::Coord(10., 15.)));
    EXPECT_TRUE(ss.posInCenteredCell(inet::Coord(15., 15.), inet::Coord(15., 10.)));
    //upper right not included.
    EXPECT_FALSE(ss.posInCenteredCell(inet::Coord(15., 15.), inet::Coord(20., 15.)));
    EXPECT_FALSE(ss.posInCenteredCell(inet::Coord(15., 15.), inet::Coord(15., 20.)));

    EXPECT_TRUE(rr.posInCenteredCell(inet::Coord(15., 30.), inet::Coord(17., 32.)));
    EXPECT_TRUE(rr.posInCenteredCell(inet::Coord(15., 30.), inet::Coord(10., 21.)));
    EXPECT_TRUE(rr.posInCenteredCell(inet::Coord(15., 30.), inet::Coord(15., 20.)));
    //upper right not included.
    EXPECT_FALSE(rr.posInCenteredCell(inet::Coord(15., 30.), inet::Coord(20., 21.)));
    EXPECT_FALSE(rr.posInCenteredCell(inet::Coord(15., 30.), inet::Coord(15., 40.)));
}

TEST_F(RegularGridInfoTest_F, cellCenterDist){
    EXPECT_EQ(ss.cellCenterDist(GridCellID(0,0), GridCellID(2,1)), sqrt(20*20+10*10));
    EXPECT_EQ(rr.cellCenterDist(GridCellID(0,0), GridCellID(2,1)), sqrt(20*20+20*20));
}

TEST_F(RegularGridInfoTest_F, maxCellDist){
    EXPECT_EQ(ss.maxCellDist(GridCellID(0,0), GridCellID(2,1)), 20.0);
    EXPECT_EQ(rr.maxCellDist(GridCellID(0,0), GridCellID(2,1)), 20.0);
}


