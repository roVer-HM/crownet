/*
 * DcDMapFileWriter.cc
 *
 *  Created on: Dec 2, 2020
 *      Author: sts
 */
#include <functional>
#include <sstream>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"

#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"
#include "crownet/common/RegularGridInfo.h"

using namespace crownet;

namespace {

DcdFactoryProvider f = DcdFactoryProvider(
        inet::Coord(.0, .0),
        inet::Coord(10.0, 10.0),
        1.0
);
RegularGridInfo  grid = f.grid;
std::shared_ptr<RegularDcdMapFactory> dcdFactory = f.dcdFactory;

}

class DcDMapFileWriterTest : public BaseOppTest{
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DcDMapFileWriterTest()
      : mapEmpty(dcdFactory->create_shared_ptr(1)),
        mapLocal(dcdFactory->create_shared_ptr(2)),
        mapFull(dcdFactory->create_shared_ptr(3)) {}

  void incr(std::shared_ptr<RegularDcdMap> map, double x, double y, int i, double t) {
    auto e = map->getEntry<GridGlobalEntry>(traci::TraCIPosition(x, y));
    e->incrementCount(t);
    e->nodeIds.insert(IntIdentifer(i));
  }
  void update(std::shared_ptr<RegularDcdMap> map, int x, int y, int id, int count, double t) {
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id));
    map->setEntry(GridCellID(x, y), std::move(e));
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
    update(mapFull, 6, 3, 808, 9, 12.0);
  }

 protected:
  std::shared_ptr<RegularDcdMap> mapEmpty;
  std::shared_ptr<RegularDcdMap> mapLocal;
  std::shared_ptr<RegularDcdMap> mapFull;
};

// todo WriterTest csv <<<

TEST_F(DcDMapFileWriterTest, print_header) {
  std::stringstream out;
  RegularDcdMapValuePrinter p(mapFull);
  p.writeHeaderTo(out, "; ");
  std::string header =
      "simtime; x; y; count; measured_t; received_t; source; selection; "
      "sourceHost; sourceEntry; hostEntry; own_cell\n";
  EXPECT_STREQ(out.str().c_str(), header.c_str());
}



TEST_F(DcDMapFileWriterTest, computeValues_idenpotent) {
  std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
  mapFull->setOwnerCell(GridCellID(3, 3));

  setSimTime(1.4);
  mapFull->computeValues(ymf_v);
  auto sourceOfSelected = mapFull->getCell(GridCellID(6, 3)).val()->getSource();
  EXPECT_EQ(sourceOfSelected.value(), 803);

  setSimTime(10.4);
  mapFull->getCell(GridCellID(6, 3)).get(803)->reset(omnetpp::simTime());
  mapFull->computeValues(ymf_v);
  sourceOfSelected = mapFull->getCell(GridCellID(6, 3)).val()->getSource();
  EXPECT_EQ(sourceOfSelected.value(), 805);

  // resetting 805 but no time increment must not change the selected value
  mapFull->getCell(GridCellID(6, 3)).get(805)->reset(omnetpp::simTime());
  mapFull->computeValues(ymf_v);
  sourceOfSelected = mapFull->getCell(GridCellID(6, 3)).val()->getSource();
  EXPECT_EQ(sourceOfSelected.value(), 805);
}





TEST_F(DcDMapFileWriterTest, printglobal) {
  std::stringstream out;
  RegularDcdMapGlobalPrinter p(mapLocal);
  mapLocal->setOwnerCell(GridCellID(3, 3));

  setSimTime(1.4);
  p.writeTo(out, "; ");

  setSimTime(10.4);
  mapLocal->getEntry<GridGlobalEntry>(traci::TraCIPosition(4.3, 4.1))->incrementCount(simTime());
  mapLocal->getEntry<GridGlobalEntry>(traci::TraCIPosition(4.3, 4.1))->nodeIds.insert(301);
  p.writeTo(out, "; ");

  setSimTime(20.4);
  mapLocal->getEntry<>(traci::TraCIPosition(4.3, 4.1))->incrementCount(simTime());
  mapLocal->getEntry<GridGlobalEntry>(traci::TraCIPosition(4.3, 4.1))->nodeIds.insert(302);
  mapLocal->getCell(GridCellID(1, 1)).getLocal()->reset(simTime());
  p.writeTo(out, "; ");

  std::string ret =
      "1.4; 1; 1; 2; 30; 30; 2; ; 0; 100, 101\n"
      "1.4; 3; 3; 3; 32; 32; 2; ; 1; 200, 201, 202\n"
      "1.4; 4; 4; 1; 34; 34; 2; ; 0; 300\n"
      "10.4; 1; 1; 2; 30; 30; 2; ; 0; 100, 101\n"
      "10.4; 3; 3; 3; 32; 32; 2; ; 1; 200, 201, 202\n"
      "10.4; 4; 4; 2; 10.4; 10.4; 2; ; 0; 300, 301\n"
      "20.4; 3; 3; 3; 32; 32; 2; ; 1; 200, 201, 202\n"
      "20.4; 4; 4; 3; 20.4; 20.4; 2; ; 0; 300, 301, 302\n";

  EXPECT_STREQ(out.str().c_str(), ret.c_str());
}

class DcDMapFileWriterNoGlobalEntryTest : public BaseOppTest, public DcdFactoryProvider {
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DcDMapFileWriterNoGlobalEntryTest()
      : mapEmpty(dcdFactory->create_shared_ptr(1)),
        mapLocal(dcdFactory->create_shared_ptr(2)),
        mapFull(dcdFactory->create_shared_ptr(3)) {}

  void incr(std::shared_ptr<RegularDcdMap> map, double x, double y, int i, double t) {
    auto e = map->getEntry<Entry>(traci::TraCIPosition(x, y));
    e->incrementCount(t);
  }
  void update(std::shared_ptr<RegularDcdMap> map, int x, int y, int id, int count, double t) {
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id));
    map->setEntry(GridCellID(x, y), std::move(e));
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
    update(mapFull, 6, 3, 808, 9, 12.0);
  }

 protected:
  std::shared_ptr<RegularDcdMap> mapEmpty;
  std::shared_ptr<RegularDcdMap> mapLocal;
  std::shared_ptr<RegularDcdMap> mapFull;
};

TEST_F(DcDMapFileWriterNoGlobalEntryTest, print_all_valid) {
  std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
  std::stringstream out;
  RegularDcdMapValuePrinter p(mapFull);
  mapFull->setOwnerCell(GridCellID(3, 3));

  setSimTime(1.4);
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(10.4);
  // must use different measure
  mapFull->getCell(GridCellID(6, 3)).get(803)->reset(omnetpp::simTime());
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(20.4);
  // will not change anything because '3' is not ymf there is a
  mapFull->getCell(GridCellID(1, 1)).get(3)->reset(omnetpp::simTime());
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(30.4);
  // new cell should show up
  update(mapFull, 5, 5, 333, 3, 30.4);
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(40.4);
  // reseting all must not add new lines
  ResetVisitor r(simTime());
  mapFull->visitCells(r);
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  std::string ret =
      "1.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "1.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "1.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "1.4; 6; 3; 5; 33; 33; 803; ymf; 0; 0; 0; 0\n"
      "10.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "10.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "10.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "10.4; 6; 3; 7; 31; 31; 805; ymf; 0; 0; 0; 0\n"
      "20.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "20.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "20.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "20.4; 6; 3; 7; 31; 31; 805; ymf; 0; 0; 0; 0\n"
      "30.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "30.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "30.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "30.4; 5; 5; 3; 30.4; 30.4; 333; ymf; 0; 0; 0; 0\n"
      "30.4; 6; 3; 7; 31; 31; 805; ymf; 0; 0; 0; 0\n";
  ;

  EXPECT_STREQ(out.str().c_str(), ret.c_str());
}

TEST_F(DcDMapFileWriterNoGlobalEntryTest, print_duplicate_selections) {
    /*
     *  For each cell only one entry can be the 'selected'
     *  The selection entry in the Entry object must be reset between
     *  calls to MAP.computeValues() to ensure only on 'selected' value
     *  to be set if the hole map is printed. with RegularDcdMapAllPrinter
     */
    std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
    std::stringstream out;
    RegularDcdMapAllPrinter p(mapFull);
    mapFull->setOwnerCell(GridCellID(3, 3));

    setSimTime(1.4);
    mapFull->computeValues(ymf_v);
    //source of Cell[3,3] must be id=3
    auto sourceOfSelected = mapFull->getCell(GridCellID(3,3)).val()->getSource();
    EXPECT_EQ(sourceOfSelected.value(), 3);
    p.writeTo(out, "; ");


    setSimTime(10.4);
    // set time to t=33 previous time for [3,3] was 32
    // this should now be the selected value
    update(mapFull, 3, 3, 555, 10, 33.3); // x,y,id,count,time
    mapFull->computeValues(ymf_v);
    sourceOfSelected = mapFull->getCell(GridCellID(3,3)).val()->getSource();
    EXPECT_EQ(sourceOfSelected.value(), 555);
    p.writeTo(out, "; ");

    std::string ret =
            "1.4; 1; 1; 2; 30; 30; 3; ; 0; 0; 0; 0\n"
            "1.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
            "1.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
            "1.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
            "1.4; 4; 4; 3; 32; 32; 801; ; 0; 0; 0; 0\n"
            "1.4; 6; 3; 5; 33; 33; 803; ymf; 0; 0; 0; 0\n"
            "1.4; 6; 3; 7; 31; 31; 805; ; 0; 0; 0; 0\n"
            "1.4; 6; 3; 9; 12; 12; 808; ; 0; 0; 0; 0\n"
            "10.4; 1; 1; 2; 30; 30; 3; ; 0; 0; 0; 0\n"
            "10.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
            "10.4; 3; 3; 3; 32; 32; 3; ; 0; 0; 0; 1\n" // << no 'ymf' here!
            "10.4; 3; 3; 10; 33.3; 33.3; 555; ymf; 0; 0; 0; 1\n"
            "10.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
            "10.4; 4; 4; 3; 32; 32; 801; ; 0; 0; 0; 0\n"
            "10.4; 6; 3; 5; 33; 33; 803; ymf; 0; 0; 0; 0\n"
            "10.4; 6; 3; 7; 31; 31; 805; ; 0; 0; 0; 0\n"
            "10.4; 6; 3; 9; 12; 12; 808; ; 0; 0; 0; 0\n";

    EXPECT_STREQ(out.str().c_str(), ret.c_str());
//    std::cout << out.str() << std::endl;
}
TEST_F(DcDMapFileWriterNoGlobalEntryTest, printall_all_valid) {
  std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
  std::stringstream out;
  RegularDcdMapAllPrinter p(mapFull);
  mapFull->setOwnerCell(GridCellID(3, 3));

  setSimTime(1.4);
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(10.4);
  // must use different ymf measure and is not in print out because
  // it is invalid.
  mapFull->getCell(GridCellID(6, 3)).get(803)->reset(omnetpp::simTime());
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(20.4);
  // will not change anything because '3' is not ymf there is a
  mapFull->getCell(GridCellID(1, 1)).get(3)->reset(omnetpp::simTime());
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(30.4);
  // new cell should show up
  update(mapFull, 5, 5, 333, 3, 30.4);
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  setSimTime(40.4);
  // reseting all must not add new lines
  ResetVisitor r(simTime());
  mapFull->visitCells(r);
  mapFull->computeValues(ymf_v);
  p.writeTo(out, "; ");

  std::string ret =
      "1.4; 1; 1; 2; 30; 30; 3; ; 0; 0; 0; 0\n"
      "1.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "1.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "1.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "1.4; 4; 4; 3; 32; 32; 801; ; 0; 0; 0; 0\n"
      "1.4; 6; 3; 5; 33; 33; 803; ymf; 0; 0; 0; 0\n"
      "1.4; 6; 3; 7; 31; 31; 805; ; 0; 0; 0; 0\n"
      "1.4; 6; 3; 9; 12; 12; 808; ; 0; 0; 0; 0\n"
      "10.4; 1; 1; 2; 30; 30; 3; ; 0; 0; 0; 0\n"
      "10.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "10.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "10.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "10.4; 4; 4; 3; 32; 32; 801; ; 0; 0; 0; 0\n"
      "10.4; 6; 3; 7; 31; 31; 805; ymf; 0; 0; 0; 0\n"
      "10.4; 6; 3; 9; 12; 12; 808; ; 0; 0; 0; 0\n"
      "20.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "20.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "20.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "20.4; 4; 4; 3; 32; 32; 801; ; 0; 0; 0; 0\n"
      "20.4; 6; 3; 7; 31; 31; 805; ymf; 0; 0; 0; 0\n"
      "20.4; 6; 3; 9; 12; 12; 808; ; 0; 0; 0; 0\n"
      "30.4; 1; 1; 4; 31; 31; 800; ymf; 0; 0; 0; 0\n"
      "30.4; 3; 3; 3; 32; 32; 3; ymf; 0; 0; 0; 1\n"
      "30.4; 4; 4; 2; 34; 34; 3; ymf; 0; 0; 0; 0\n"
      "30.4; 4; 4; 3; 32; 32; 801; ; 0; 0; 0; 0\n"
      "30.4; 5; 5; 3; 30.4; 30.4; 333; ymf; 0; 0; 0; 0\n"
      "30.4; 6; 3; 7; 31; 31; 805; ymf; 0; 0; 0; 0\n"
      "30.4; 6; 3; 9; 12; 12; 808; ; 0; 0; 0; 0\n";

  EXPECT_STREQ(out.str().c_str(), ret.c_str());
}
