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
#include "crownet/crownet_testutil.h"
#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "inet/common/geometry/common/Coord.h"

using namespace crownet;

class GridCellIdKeyProvider_CellsInRange : public MapTest{

    void SetUp() override {
        keyProvider = this->dcdFactory->getCellKeyProvider();
    }

protected:

    std::shared_ptr<GridCellIDKeyProvider> keyProvider; //10x10
};

TEST_F(GridCellIdKeyProvider_CellsInRange, one_cell_in_range_dist_smaller_than_cell){

    //center of map
    inet::Coord pos {5.4, 4.4};
    auto vec = keyProvider->getCellsInRadius(pos, 0.1);

    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0], GridCellID(5, 5));

}

TEST_F(GridCellIdKeyProvider_CellsInRange, cells_in_range_dist_equal_cell_size_center_map){

    //center of map
    inet::Coord pos {5.4, 4.4};// off center OMNeT Coords upper left
    auto vec = keyProvider->getCellsInRadius(pos, 0.5); // cell size 1x1 meter

    EXPECT_EQ(vec.size(), 5);

    std::vector<GridCellID> vec_expected = {GridCellID{4, 5}, GridCellID{5, 4}, GridCellID{5, 5}, GridCellID{5, 6}, GridCellID{6, 5}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, cells_in_range_dist_equal_cell_size_lower_left_map){

    //center of map
    inet::Coord pos {0.1, 9.9};// off center OMNeT Coords upper left
    auto vec = keyProvider->getCellsInRadius(pos, 0.5); // cell size 1x1 meter

    EXPECT_EQ(vec.size(), 3);

    std::vector<GridCellID> vec_expected = {GridCellID{0, 0}, GridCellID{0, 1}, GridCellID{1, 0}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, cells_in_range_dist_equal_cell_size_upper_left_map){

    //center of map
    inet::Coord pos {0.1, 0.1};// off center OMNeT Coords upper left
    auto vec = keyProvider->getCellsInRadius(pos, 0.5); // cell size 1x1 meter

    EXPECT_EQ(vec.size(), 3);

    std::vector<GridCellID> vec_expected = {GridCellID{0, 8}, GridCellID{0, 9}, GridCellID{1, 9}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, cells_in_range_dist_equal_cell_size_upper_right_map){

    //center of map
    inet::Coord pos {9.9, 0.1};// off center OMNeT Coords upper left
    auto vec = keyProvider->getCellsInRadius(pos, 0.5); // cell size 1x1 meter

    EXPECT_EQ(vec.size(), 3);

    std::vector<GridCellID> vec_expected = {GridCellID{8, 9}, GridCellID{9, 8}, GridCellID{9, 9}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, cells_in_range_dist_equal_cell_size_lower_right_map){

    //center of map
    inet::Coord pos {9.9, 9.9};// off center OMNeT Coords upper left
    auto vec = keyProvider->getCellsInRadius(pos, 0.5); // cell size 1x1 meter

    EXPECT_EQ(vec.size(), 3);

    std::vector<GridCellID> vec_expected = {GridCellID{8, 0}, GridCellID{9, 0}, GridCellID{9, 1}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, eight_cell_neighborhood_center){

    //center of map
    inet::Coord pos {5.5, 4.5};
    auto vec = keyProvider->getCellsInRadius(pos, 0.8); // enclosing circle radius

    EXPECT_EQ(vec.size(), 9);
    std::vector<GridCellID> vec_expected = {
            GridCellID{4, 4}, GridCellID{4, 5}, GridCellID{4, 6},
            GridCellID{5, 4}, GridCellID{5, 5}, GridCellID{5, 6},
            GridCellID{6, 4}, GridCellID{6, 5}, GridCellID{6, 6}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, eight_cell_neighborhood_lower_left){

    //center of map
    inet::Coord pos {0.1, 9.9};
    auto vec = keyProvider->getCellsInRadius(pos, 0.8); // enclosing circle radius

    EXPECT_EQ(vec.size(), 4);
    std::vector<GridCellID> vec_expected = {
            GridCellID{0, 0}, GridCellID{0, 1},
            GridCellID{1, 0}, GridCellID{1, 1}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, eight_cell_neighborhood_upper_left){

    //center of map
    inet::Coord pos {0.1, 0.1};
    auto vec = keyProvider->getCellsInRadius(pos, 0.8); // enclosing circle radius

    EXPECT_EQ(vec.size(), 4);
    std::vector<GridCellID> vec_expected = {
            GridCellID{0, 8}, GridCellID{0, 9},
            GridCellID{1, 8}, GridCellID{1, 9}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, eight_cell_neighborhood_upper_right){

    //center of map
    inet::Coord pos {9.9, 0.1};
    auto vec = keyProvider->getCellsInRadius(pos, 0.8); // enclosing circle radius

    EXPECT_EQ(vec.size(), 4);
    std::vector<GridCellID> vec_expected = {
            GridCellID{8, 8}, GridCellID{8, 9},
            GridCellID{9, 8}, GridCellID{9, 9}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, eight_cell_neighborhood_lower_right){

    //center of map
    inet::Coord pos {9.9, 9.9};
    auto vec = keyProvider->getCellsInRadius(pos, 0.8); // enclosing circle radius

    EXPECT_EQ(vec.size(), 4);
    std::vector<GridCellID> vec_expected = {
            GridCellID{8, 0}, GridCellID{8, 1},
            GridCellID{9, 0}, GridCellID{9, 1}};
    cmp_vec(vec, vec_expected);
}

TEST_F(GridCellIdKeyProvider_CellsInRange, all_cells){

    //center of map
    inet::Coord pos {5.0, 5.0};
    auto vec = keyProvider->getCellsInRadius(pos, 1000); // enclosing circle radius

    EXPECT_EQ(vec.size(), 100);
}


class GridCellIdKeyProvider_F : public BaseOppTest {
public:
    GridCellIdKeyProvider_F(){
        DcdFactoryProvider provider_s = DcdFactoryProvider(
                inet::Coord(0.0, 0.0),
                inet::Coord(100.0, 100.0),
                inet::Coord(10.0, 10.0)
        );
        s = std::make_shared<GridCellIDKeyProvider>(provider_s.converter);

        DcdFactoryProvider provider_r = DcdFactoryProvider(
                inet::Coord(0.0, 0.0),
                inet::Coord(100.0, 200.0),
                inet::Coord(10.0, 20.0)
        );
        r = std::make_shared<GridCellIDKeyProvider>(provider_r.converter);

    }

    protected:
    std::shared_ptr<GridCellIDKeyProvider> s;
    std::shared_ptr<GridCellIDKeyProvider> r;
};


TEST_F(GridCellIdKeyProvider_F, traciCoordToCell){
    EXPECT_EQ(s->getCellKey(traci::TraCIPosition{1.0, 1.0}), GridCellID(0,0));
    EXPECT_EQ(s->getCellKey(traci::TraCIPosition{5.0, 95.0}), GridCellID(0,9));
    EXPECT_EQ(s->getCellKey(traci::TraCIPosition{0.0, 0.0}), GridCellID(0,0));
    EXPECT_EQ(s->getCellKey(traci::TraCIPosition{20.0, 10.0}), GridCellID(2,1));
    EXPECT_EQ(s->getCellKey(traci::TraCIPosition{19.9, 10.0}), GridCellID(1,1));

    EXPECT_EQ(s->getCellKey(traci::TraCIPosition{1.0, 15.0}), GridCellID(0,1));
    EXPECT_EQ(r->getCellKey(traci::TraCIPosition{1.0, 15.0}), GridCellID(0,0)); // rectangle cells!
}

TEST_F(GridCellIdKeyProvider_F, inetCoordToCell){
    EXPECT_EQ(s->getCellKey(inet::Coord{1.0, 99.0}), GridCellID(0,0));
    EXPECT_EQ(s->getCellKey(inet::Coord{5.0, 5.0}), GridCellID(0,9));
    EXPECT_EQ(s->getCellKey(inet::Coord{0.0, 100.0}), GridCellID(0,0));
    EXPECT_EQ(s->getCellKey(inet::Coord{20.0, 90.0}), GridCellID(2,1));
    EXPECT_EQ(s->getCellKey(inet::Coord{19.9, 90.0}), GridCellID(1,1));

    EXPECT_EQ(s->getCellKey(inet::Coord{1.0, 85.0}), GridCellID(0,1));
    EXPECT_EQ(r->getCellKey(inet::Coord{1.0, 185.0}), GridCellID(0,0)); // rectangle cells!
}

TEST_F(GridCellIdKeyProvider_F, traciCoordChangedCell){
    EXPECT_FALSE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{1.0, 1.0}));
    EXPECT_FALSE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{5.0, 9.0}));
    EXPECT_FALSE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{1.0, 1.0}));
    EXPECT_FALSE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{5.0, 15.0}));

    EXPECT_TRUE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{10.0, 1.0}));
    EXPECT_TRUE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{1.0, 10.0}));
    EXPECT_TRUE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{10.0, 1.0}));
    EXPECT_TRUE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{1.0, 20.0}));

    EXPECT_TRUE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{15.0, 5.0}));
    EXPECT_TRUE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{5.0, 15.0}));
    EXPECT_TRUE(s->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{15.0, 15.0}));

    EXPECT_TRUE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{15.0, 5.0}));
    EXPECT_TRUE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{5.0, 25.0}));
    EXPECT_TRUE(r->changedCell(traci::TraCIPosition{1.0, 1.0}, traci::TraCIPosition{15.0, 25.0}));
}

TEST_F(GridCellIdKeyProvider_F, inetCoordChangedCell){
    EXPECT_FALSE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{1.0, 99.0}));
    EXPECT_FALSE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{5.0, 91.0}));
    EXPECT_FALSE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{1.0, 199.0}));
    EXPECT_FALSE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{5.0, 185.0}));

    EXPECT_TRUE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{10.0, 99.0}));
    EXPECT_TRUE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{1.0, 90.0}));
    EXPECT_TRUE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{10.0, 199.0}));
    EXPECT_TRUE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{1.0, 180.0}));

    EXPECT_TRUE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{15.0, 95.0}));
    EXPECT_TRUE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{5.0, 85.0}));
    EXPECT_TRUE(s->changedCell(inet::Coord{1.0, 99.0}, inet::Coord{15.0, 85.0}));

    EXPECT_TRUE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{15.0, 195.0}));
    EXPECT_TRUE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{5.0, 175.0}));
    EXPECT_TRUE(r->changedCell(inet::Coord{1.0, 199.0}, inet::Coord{15.0, 175.0}));
}

TEST_F(GridCellIdKeyProvider_F, gridInfo){
    EXPECT_EQ(s->getCellSize(), Coord(10.0, 10.0, 0.0));
    EXPECT_EQ(s->getGridSize(), Coord(100.0, 100.0, 0.0));

    EXPECT_EQ(r->getCellSize(), Coord(10.0, 20.0, 0.0));
    EXPECT_EQ(r->getGridSize(), Coord(100.0, 200.0, 0.0));
}

TEST_F(GridCellIdKeyProvider_F, dist1){
    double dist1 = s->cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    double dist2 = s->cellCenterDist(GridCellID(2,8), GridCellID(2,2));
    EXPECT_EQ(dist1, dist2);

    double dist3 = r->cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    double dist4 = r->cellCenterDist(GridCellID(2,8), GridCellID(2,2));
    EXPECT_EQ(dist3, dist4);

}

TEST_F(GridCellIdKeyProvider_F, dist2){
    double dist1 = s->cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist1, 60.0);

    double dist2 = r->cellCenterDist(GridCellID(2,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist2, 120.0);
}

TEST_F(GridCellIdKeyProvider_F, dist3){
    double dist1 = s->cellCenterDist(GridCellID(0,0), GridCellID(3,4)); // 30x40
    EXPECT_DOUBLE_EQ(dist1, std::sqrt(30*30 + 40*40));

    double dist2 = r->cellCenterDist(GridCellID(0,0), GridCellID(3,4)); // 30x80
    EXPECT_DOUBLE_EQ(dist2, std::sqrt(30*30 + 80*80));
}


TEST_F(GridCellIdKeyProvider_F, dist4){
    double dist1 = s->maxCellDist(GridCellID(2,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist1, 60.0);
    double dist2 = s->maxCellDist(GridCellID(3,2), GridCellID(2,8));
    EXPECT_DOUBLE_EQ(dist2, 60.0); // still 60. because longest range (x or y) is used

    double dist4 = r->maxCellDist(GridCellID(2,2), GridCellID(3,3));
    EXPECT_EQ(dist4, 20.0);
}


