/*
 * DcDMapTest.cc
 *
 *  Created on: Nov 24, 2020
 *      Author: sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"

using namespace crownet;

namespace {
RegularDcdMapFactory dcdFactory{std::make_pair(1.0, 1.0),
                                std::make_pair(10, 10)};
}

class RegularDcDMapTest : public BaseOppTest {
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  RegularDcDMapTest()
      : mapEmpty(dcdFactory.create(1)),
        mapLocal(dcdFactory.create(2)),
        mapFull(dcdFactory.create(3)) {}

  void incr(RegularDcdMap& map, double x, double y, int i, double t) {
    map.incrementLocal(traci::TraCIPosition(x, y), IntIdentifer(i), t);
  }
  void update(RegularDcdMap& map, int x, int y, int id, int count, double t) {
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id));
    map.update(GridCellID(x, y), std::move(e));
  }

  void SetUp() override {
    // [1, 1] count 2
    incr(mapLocal, 1.2, 1.2, 100, 30.0);
    incr(mapLocal, 1.5, 1.5, 101, 30.0);
    // [3, 3] count 3
    incr(mapLocal, 3.2, 3.2, 200, 32.0);
    incr(mapLocal, 3.5, 3.5, 201, 32.0);
    incr(mapLocal, 3.5, 3.5, 202, 32.0);
    // [4, 4] count 1
    incr(mapLocal, 4.2, 4.5, 300, 34.0);
    // local neighbors [ 100, 101, 200, 201, 202, 300] #6

    // [1, 1] Local = 2 / Ohter = 4
    incr(mapFull, 1.2, 1.2, 100, 30.0);
    incr(mapFull, 1.5, 1.5, 101, 30.0);
    update(mapFull, 1, 1, 800, 4, 31.0);
    // [3, 3] count 3 / Other = 3
    incr(mapFull, 3.2, 3.2, 200, 32.0);
    incr(mapFull, 3.5, 3.5, 201, 32.0);
    incr(mapFull, 3.5, 3.5, 202, 32.0);
    update(mapFull, 4, 4, 801, 3, 32.0);
    // [4, 4] count 2 / Other = 0
    incr(mapFull, 4.2, 4.5, 300, 34.0);
    incr(mapFull, 4.2, 4.5, 301, 34.0);
    // [6, 3] count 0 / Other = [5, 7, 9]
    update(mapFull, 6, 3, 803, 5, 33.0);
    update(mapFull, 6, 3, 805, 7, 31.0);
    update(mapFull, 6, 3, 808, 9, 39.0);
  }

 protected:
  RegularDcdMap mapEmpty;
  RegularDcdMap mapLocal;
  RegularDcdMap mapFull;
};

TEST_F(RegularDcDMapTest, getOwner) {
  EXPECT_EQ(IntIdentifer(1), mapEmpty.getOwnerId());
  EXPECT_EQ(IntIdentifer(2), mapLocal.getOwnerId());
  EXPECT_EQ(IntIdentifer(3), mapFull.getOwnerId());
}

TEST_F(RegularDcDMapTest, getCells_Empty) {
  EXPECT_EQ(0, mapEmpty.getCells().size());
}

TEST_F(RegularDcDMapTest, getCells_Local) {
  EXPECT_EQ(3, mapLocal.getCells().size());
}

TEST_F(RegularDcDMapTest, getCells_Full) {
  EXPECT_EQ(4, mapFull.getCells().size());
}

TEST_F(RegularDcDMapTest, hasCells_Fail) {
  EXPECT_FALSE(mapEmpty.hasCell(GridCellID(3, 3)));
  EXPECT_FALSE(mapLocal.hasCell(GridCellID(99, 3)));
  EXPECT_FALSE(mapFull.hasCell(GridCellID(99, 3)));
}

TEST_F(RegularDcDMapTest, hasCells_Sucess) {
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(1, 1)));
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(3, 3)));
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(4, 4)));

  EXPECT_TRUE(mapFull.hasCell(GridCellID(1, 1)));
  EXPECT_TRUE(mapFull.hasCell(GridCellID(3, 3)));
  EXPECT_TRUE(mapFull.hasCell(GridCellID(4, 4)));
  EXPECT_TRUE(mapFull.hasCell(GridCellID(6, 3)));
}

TEST_F(RegularDcDMapTest, hasDataFrom_noCells) {
  EXPECT_FALSE(mapEmpty.hasDataFrom(GridCellID(3, 3), IntIdentifer(4)));
  // test must not create new cells / entires
  EXPECT_EQ(0, mapEmpty.getCells().size());
}

TEST_F(RegularDcDMapTest, hasDataFrom_withCellButNoEntry) {
  // cell exists but no entry for node identifier
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(1, 1)));
  // 1 entry before test
  EXPECT_EQ(1, mapLocal.getCells()[GridCellID(1, 1)].getData().size());

  EXPECT_FALSE(mapLocal.hasDataFrom(GridCellID(1, 1), IntIdentifer(4)));
  // test must not create new cell
  EXPECT_EQ(3, mapLocal.getCells().size());
  // test must not create new entry in cell
  EXPECT_EQ(1, mapLocal.getCells()[GridCellID(1, 1)].getData().size());
}

TEST_F(RegularDcDMapTest, hasDataFrom_SuccessLocal) {
  // own ID
  EXPECT_TRUE(mapLocal.hasDataFrom(GridCellID(1, 1), IntIdentifer(2)));
}

TEST_F(RegularDcDMapTest, hasDataFrom_SuccessMultiple) {
  // own ID
  // no local
  EXPECT_FALSE(mapFull.hasDataFrom(GridCellID(6, 3), IntIdentifer(3)));
  EXPECT_TRUE(mapFull.hasDataFrom(GridCellID(6, 3), IntIdentifer(803)));
  EXPECT_TRUE(mapFull.hasDataFrom(GridCellID(6, 3), IntIdentifer(805)));
  EXPECT_TRUE(mapFull.hasDataFrom(GridCellID(6, 3), IntIdentifer(808)));
}

TEST_F(RegularDcDMapTest, str) {
  EXPECT_STREQ("{map_owner: 1 cell_count: 0 local_cell_count: 0}", mapEmpty.str().c_str());
  EXPECT_STREQ("{map_owner: 2 cell_count: 3 local_cell_count: 3}", mapLocal.str().c_str());
  EXPECT_STREQ("{map_owner: 3 cell_count: 4 local_cell_count: 3}", mapFull.str().c_str());
}

TEST_F(RegularDcDMapTest, getMissingCell) {
  EXPECT_FALSE(mapLocal.hasCell(GridCellID(3, 2)));
  auto& cell = mapLocal.getCell(GridCellID(3, 2));
  EXPECT_EQ(GridCellID(3, 2), cell.getCellId());
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(3, 2)));
}

TEST_F(RegularDcDMapTest, getExistingCell) {
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(1, 1)));
  auto& cell = mapLocal.getCell(GridCellID(1, 1));
  EXPECT_EQ(GridCellID(1, 1), cell.getCellId());
}

TEST_F(RegularDcDMapTest, createMissingCell) {
  EXPECT_FALSE(mapLocal.hasCell(GridCellID(3, 2)));
  auto& cell = mapLocal.createCell(GridCellID(3, 2));
  EXPECT_EQ(GridCellID(3, 2), cell.getCellId());
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(3, 2)));
}

TEST_F(RegularDcDMapTest, createExistingCell) {
  // do not create new object if it exists.
  EXPECT_TRUE(mapLocal.hasCell(GridCellID(1, 1)));
  auto& cell = mapLocal.createCell(GridCellID(1, 1));
  EXPECT_EQ(cell, mapLocal.getCells()[GridCellID(1, 1)]);
}

TEST_F(RegularDcDMapTest, setOwnerId) {
  EXPECT_EQ(IntIdentifer(1), mapEmpty.getOwnerId());
  mapEmpty.setOwnerId(11);
  EXPECT_EQ(IntIdentifer(11), mapEmpty.getOwnerId());
}

// update / incrementLocal  correct in Setup compare string
TEST_F(RegularDcDMapTest, strFull) {
  std::string s1 = "{map_owner: 1 cell_count: 0 local_cell_count: 0}\n";
  EXPECT_STREQ(s1.c_str(), mapEmpty.strFull().c_str());

  std::string s2 =
      "{map_owner: 2 cell_count: 3 local_cell_count: 3}\n"
      "  {cell_id: [1,1] owner_id: 2}\n"
      "    2 --> Count: 2| meas_t: 30| recv_t: 30| valid: 1| node_ids: {100, "
      "101}\n"
      "  {cell_id: [3,3] owner_id: 2}\n"
      "    2 --> Count: 3| meas_t: 32| recv_t: 32| valid: 1| node_ids: {200, "
      "201, 202}\n"
      "  {cell_id: [4,4] owner_id: 2}\n"
      "    2 --> Count: 1| meas_t: 34| recv_t: 34| valid: 1| node_ids: {300}\n";
  EXPECT_STREQ(s2.c_str(), mapLocal.strFull().c_str());

  std::string s3 =
      "{map_owner: 3 cell_count: 4 local_cell_count: 3}\n"
      "  {cell_id: [1,1] owner_id: 3}\n"
      "    3 --> Count: 2| meas_t: 30| recv_t: 30| valid: 1| node_ids: "
      "{100, 101}\n"
      "    800 --> Count: 4| meas_t: 31| recv_t: 31| valid: 1\n"
      "  {cell_id: [3,3] owner_id: 3}\n"
      "    3 --> Count: 3| meas_t: 32| recv_t: 32| valid: 1| node_ids: "
      "{200, 201, 202}\n"
      "  {cell_id: [4,4] owner_id: 3}\n"
      "    3 --> Count: 2| meas_t: 34| recv_t: 34| valid: 1| node_ids: "
      "{300, 301}\n"
      "    801 --> Count: 3| meas_t: 32| recv_t: 32| valid: 1\n"
      "  {cell_id: [6,3] owner_id: 3}\n"
      "    803 --> Count: 5| meas_t: 33| recv_t: 33| valid: 1\n"
      "    805 --> Count: 7| meas_t: 31| recv_t: 31| valid: 1\n"
      "    808 --> Count: 9| meas_t: 39| recv_t: 39| valid: 1\n";
  EXPECT_STREQ(s3.c_str(), mapFull.strFull().c_str());
}

TEST_F(RegularDcDMapTest, isInNeighborhood_True) {
  EXPECT_TRUE(mapLocal.isInNeighborhood(100));
  EXPECT_TRUE(mapLocal.isInNeighborhood(101));
  EXPECT_TRUE(mapLocal.isInNeighborhood(200));
  EXPECT_TRUE(mapLocal.isInNeighborhood(202));
  EXPECT_TRUE(mapLocal.isInNeighborhood(201));
  EXPECT_TRUE(mapLocal.isInNeighborhood(300));
}

TEST_F(RegularDcDMapTest, isInNeighborhood_False) {
  EXPECT_FALSE(mapLocal.isInNeighborhood(999));
}

TEST_F(RegularDcDMapTest, sizeOfNeighborhood_Empty) {
  EXPECT_EQ(0, mapEmpty.sizeOfNeighborhood());
}

TEST_F(RegularDcDMapTest, sizeOfNeighborhood_NotEmpty) {
  EXPECT_EQ(6, mapLocal.sizeOfNeighborhood());
  EXPECT_EQ(7, mapFull.sizeOfNeighborhood());
}

TEST_F(RegularDcDMapTest, clearNeighborhood) {
  EXPECT_EQ(0, mapEmpty.sizeOfNeighborhood());
  EXPECT_EQ(6, mapLocal.sizeOfNeighborhood());
  EXPECT_EQ(7, mapFull.sizeOfNeighborhood());
  mapEmpty.clearNeighborhood();
  mapLocal.clearNeighborhood();
  mapFull.clearNeighborhood();
  EXPECT_EQ(0, mapEmpty.sizeOfNeighborhood());
  EXPECT_EQ(0, mapLocal.sizeOfNeighborhood());
  EXPECT_EQ(0, mapFull.sizeOfNeighborhood());
}

TEST_F(RegularDcDMapTest, removeFromNeighborhood_existing) {
  EXPECT_TRUE(mapLocal.isInNeighborhood(100));
  mapLocal.removeFromNeighborhood(100);
  EXPECT_FALSE(mapLocal.isInNeighborhood(100));
}
TEST_F(RegularDcDMapTest, removeFromNeighborhood_notExisting) {
  EXPECT_FALSE(mapLocal.isInNeighborhood(999));
  mapLocal.removeFromNeighborhood(999);
  EXPECT_FALSE(mapLocal.isInNeighborhood(999));
}

TEST_F(RegularDcDMapTest, moveNeighborTo_existing) {
  EXPECT_TRUE(mapLocal.isInNeighborhood(100));
  mapLocal.removeFromNeighborhood(100);
  EXPECT_FALSE(mapLocal.isInNeighborhood(100));
}
TEST_F(RegularDcDMapTest, moveNeighborTo_notExisting) {
  EXPECT_FALSE(mapLocal.isInNeighborhood(999));
  mapLocal.removeFromNeighborhood(999);
  EXPECT_FALSE(mapLocal.isInNeighborhood(999));
}

TEST_F(RegularDcDMapTest, getNeighborCell_existing) {
  auto cell = mapLocal.getNeighborCell(100);
  EXPECT_EQ(GridCellID(1, 1), cell);
}

TEST_F(RegularDcDMapTest, getNeighborCell_notExisting) {
  try {
    auto cell = mapLocal.getNeighborCell(999);
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularDcDMapTest, moveNeigborTo_existing) {
  EXPECT_EQ(GridCellID(1, 1), mapLocal.getNeighborCell(100));
  mapLocal.moveNeighborTo(100, GridCellID(4, 9));
  EXPECT_EQ(GridCellID(4, 9), mapLocal.getNeighborCell(100));
}

TEST_F(RegularDcDMapTest, moveNeigborTo_notExisting) {
  try {
    mapLocal.moveNeighborTo(999, GridCellID(4, 9));
    FAIL() << "omnetpp::cRuntimeError expected" << std::endl;
  } catch (omnetpp::cRuntimeError e) {
  } catch (...) {
    FAIL() << "unexpected throw" << std::endl;
  }
}

TEST_F(RegularDcDMapTest, addToNeighborhood_existing) {
  // add existing same as moveNeigborTo
  EXPECT_EQ(GridCellID(1, 1), mapLocal.getNeighborCell(100));
  mapLocal.addToNeighborhood(100, GridCellID(4, 9));
  EXPECT_EQ(GridCellID(4, 9), mapLocal.getNeighborCell(100));
}

TEST_F(RegularDcDMapTest, addToNeighborhood_notExisting) {
  EXPECT_FALSE(mapLocal.isInNeighborhood(999));
  mapLocal.addToNeighborhood(999, GridCellID(4, 9));
  EXPECT_EQ(GridCellID(4, 9), mapLocal.getNeighborCell(999));
}

/**
 * check move semantic on update
 */
TEST(RegularMap, update_move) {
  using RegularEntry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  auto m1 = std::make_shared<RegularEntry>(5, 4., 3., IntIdentifer(40));
  auto m2 = std::make_shared<RegularEntry>(3, 2., 1., IntIdentifer(50));
  auto m3 = std::make_shared<RegularEntry>(19, 18., 17., IntIdentifer(40));

  auto cellId1 = GridCellID(5, 4);
  RegularDcdMap map = dcdFactory.create(IntIdentifer(55));

  // cell must exist after update is called on it.
  EXPECT_FALSE(map.hasCell(cellId1));
  map.update(cellId1, std::move(m1));
  map.update(cellId1, std::move(m2));
  map.update(cellId1, m3);
  EXPECT_TRUE(map.hasCell(cellId1));

  auto mCount = map.getCell(cellId1).getData().size();
  EXPECT_EQ(mCount, 2);  // after two updates with different measurements
                         // 40 is updated twice

  EXPECT_FALSE(m1);
  EXPECT_FALSE(m2);
  EXPECT_TRUE(m3);  // not moved
}

TEST(RegularMap, update1) {
  using RegularEntry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  auto m1 = std::make_shared<RegularEntry>(5, 4., 3., IntIdentifer(40));
  auto m2 = std::make_shared<RegularEntry>(3, 2., 1., IntIdentifer(40));

  auto cellId1 = GridCellID(5, 4);
  RegularDcdMap map = dcdFactory.create(IntIdentifer(55));

  // cell must exist after update is called on it.
  EXPECT_FALSE(map.hasCell(cellId1));
  map.update(cellId1, m1);
  EXPECT_TRUE(map.hasCell(cellId1));
  auto m_ = map.getCell(cellId1).get(IntIdentifer(40));
  EXPECT_EQ(*m_, *m1);

  // update same cell with new value.
  map.update(cellId1, m2);
  m_ = map.getCell(cellId1).get(IntIdentifer(40));
  EXPECT_EQ(*m_, *m2);

  // only one measurement (same node_id/cell_id)
  auto mCount = map.getCell(cellId1).getData().size();
  EXPECT_EQ(mCount, 1);
}

TEST(RegularMap, update2) {
  using RegularEntry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  auto m1 = std::make_shared<RegularEntry>(5, 4., 3., IntIdentifer(40));
  auto m2 = std::make_shared<RegularEntry>(3, 2., 1., IntIdentifer(50));
  auto m3 = std::make_shared<RegularEntry>(19, 18., 17., IntIdentifer(60));
  auto m4 = std::make_shared<RegularEntry>(7, 6., 5., IntIdentifer(70));
  auto m5 = std::make_shared<RegularEntry>(7, 6., 5., IntIdentifer(80));

  auto cellId1 = GridCellID(5, 4);
  auto cellId2 = GridCellID(3, 9);
  RegularDcdMap map = dcdFactory.create(IntIdentifer(55));

  // cell must exist after update is called on it.
  EXPECT_FALSE(map.hasCell(cellId1));
  EXPECT_FALSE(map.hasCell(cellId2));

  map.update(cellId1, m1);
  map.update(cellId1, m2);
  map.update(cellId2, m3);
  map.update(cellId2, m4);
  map.update(cellId2, m5);

  // only one measurement (same node_id/cell_id)
  auto mCount1 = map.getCell(cellId1).getData().size();
  auto mCount2 = map.getCell(cellId2).getData().size();
  EXPECT_EQ(mCount1, 2);
  EXPECT_EQ(mCount2, 3);
}
