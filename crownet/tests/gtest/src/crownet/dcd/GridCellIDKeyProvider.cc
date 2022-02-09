/*
 * GridCellIDKeyProvider.cc
 *
 *  Created on: Oct 19, 2021
 *      Author: vm-sts
 */



#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "inet/common/geometry/common/Coord.h"

using namespace crownet;

class GridCellIdKeyProvider_F : public BaseOppTest {

    protected:
    GridCellIDKeyProvider s{RegularGridInfo({100.0, 100.0},{10.0, 10.0})};
    GridCellIDKeyProvider r{RegularGridInfo({100.0, 200.0},{10.0, 20.0})};
};


TEST_F(GridCellIdKeyProvider_F, coordToCell){
    EXPECT_EQ(s.getCellKey({1.0, 1.0}), GridCellID(0,0));
    EXPECT_EQ(s.getCellKey({5.0, 95.0}), GridCellID(0,9));
    EXPECT_EQ(s.getCellKey({0.0, 0.0}), GridCellID(0,0));
    EXPECT_EQ(s.getCellKey({20.0, 10.0}), GridCellID(2,1));
    EXPECT_EQ(s.getCellKey({19.9, 10.0}), GridCellID(1,1));

    EXPECT_EQ(s.getCellKey({1.0, 15.0}), GridCellID(0,1));
    EXPECT_EQ(r.getCellKey({1.0, 15.0}), GridCellID(0,0)); // rectangle cells!
}

TEST_F(GridCellIdKeyProvider_F, gridInfo){
    EXPECT_EQ(s.getCellSize(), Coord(10.0, 10.0, 0.0));
    EXPECT_EQ(s.getGridSize(), Coord(100.0, 100.0, 0.0));

    EXPECT_EQ(r.getCellSize(), Coord(10.0, 20.0, 0.0));
    EXPECT_EQ(r.getGridSize(), Coord(100.0, 200.0, 0.0));
}

TEST_F(GridCellIdKeyProvider_F, dist1){
    double dist1 = s.cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    double dist2 = s.cellCenterDist(GridCellID(2,8), GridCellID(2,2));
    EXPECT_EQ(dist1, dist2);

    double dist3 = r.cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    double dist4 = r.cellCenterDist(GridCellID(2,8), GridCellID(2,2));
    EXPECT_EQ(dist3, dist4);

}

TEST_F(GridCellIdKeyProvider_F, dist2){
    double dist1 = s.cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist1, 60.0);

    double dist2 = r.cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist2, 120.0);
}

TEST_F(GridCellIdKeyProvider_F, dist3){
    double dist1 = s.cellCenterDist(GridCellID(0,0), GridCellID(3,4)); // 30x40
    EXPECT_DOUBLE_EQ(dist1, std::sqrt(30*30 + 40*40));

    double dist2 = r.cellCenterDist(GridCellID(0,0), GridCellID(3,4)); // 30x80
    EXPECT_DOUBLE_EQ(dist2, std::sqrt(30*30 + 80*80));
}


TEST_F(GridCellIdKeyProvider_F, dist4){
    double dist1 = s.maxCellDist(GridCellID(2,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist1, 60.0);
    double dist2 = s.maxCellDist(GridCellID(2,3), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist1, 60.0); // still 60. because longest range (x or y) is used
    int dist3 = s.maxIdCellDist(GridCellID(2,3), GridCellID(4,5));
    EXPECT_EQ(dist3, 2);

    double dist4 = r.maxCellDist(GridCellID(2,2), GridCellID(3,3));
    EXPECT_EQ(dist4, 20.0);
    // one id off in both cases! but one id has a longer cellSize
    int dist5 = r.maxIdCellDist(GridCellID(2,2), GridCellID(3,3));
    EXPECT_EQ(dist5, 1);

}


