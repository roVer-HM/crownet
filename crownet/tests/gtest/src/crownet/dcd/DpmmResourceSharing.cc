/*
 * DpmmResourceSharing.cc
 *
 *  Created on: Jun 26, 2023
 *      Author: vm-sts
 */

#include <functional>
#include <sstream>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"

#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "crownet/dcd/regularGrid/RegularDcdMapPrinter.h"
#include "crownet/common/RegularGridInfo.h"


namespace {

DcdFactoryProvider f = DcdFactoryProvider(
        inet::Coord(.0, .0),
        inet::Coord(10.0, 10.0),
        1.0
);
RegularGridInfo  grid = f.grid;
std::shared_ptr<RegularDcdMapFactory> dcdFactory = f.dcdFactory;

}
using namespace crownet;

class DpmmResourceSharingIdTest : public BaseOppTest, public DcdFactoryProvider {
    /**
     * By default all other data present is from nodes in other resource sharing domains.
     */
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DpmmResourceSharingIdTest()
      : mapFull(dcdFactory->create_shared_ptr(3)) {}

  void incr(std::shared_ptr<RegularDcdMap> map, double x, double y, int i, double t) {
    auto e = map->getEntry<Entry>(traci::TraCIPosition(x, y));
    e->setResourceSharingDomainId(ownRessourceSharingDomainId);
    e->incrementCount(t);
  }
  void update(std::shared_ptr<RegularDcdMap> map, int x, int y, int id, int count, double t, int rsdid = -1) {
    auto e = std::make_shared<Entry>(count, t, t, IntIdentifer(id));
    e->setResourceSharingDomainId(rsdid);
    map->setEntry(GridCellID(x, y), std::move(e));
  }

  void SetUp() override {
    setSimTime(0.0);

    // [1, 1] Local = 2 / Other = 4
    incr(mapFull, 1.2, 1.2, 100, 30.0);
    incr(mapFull, 1.5, 1.5, 101, 30.0);
    update(mapFull, 1, 1, 800, 4, 31.0, otherRessourceSharingDomainId);
    // [3, 3] count 3 / Other = 3
    incr(mapFull, 3.2, 3.2, 200, 32.0);
    incr(mapFull, 3.5, 3.5, 201, 32.0);
    incr(mapFull, 3.5, 3.5, 202, 32.0);
    update(mapFull, 4, 4, 801, 3, 32.0, otherRessourceSharingDomainId);
    // [4, 4] count 2 / Other = 0
    incr(mapFull, 4.2, 4.5, 300, 34.0);
    incr(mapFull, 4.2, 4.5, 301, 34.0);
    // [6, 3] count 0 / Other = [5, 7, 9]
    update(mapFull, 6, 3, 803, 5, 33.0, otherRessourceSharingDomainId);
    update(mapFull, 6, 3, 805, 7, 31.0, otherRessourceSharingDomainId);
    update(mapFull, 6, 3, 808, 9, 12.0, otherRessourceSharingDomainId);
  }

 protected:
  int ownRessourceSharingDomainId = 5;
  int otherRessourceSharingDomainId = 3;
  std::shared_ptr<RegularDcdMap> mapFull;
};




TEST_F(DpmmResourceSharingIdTest, ApplyRessourceSharingDomainIdVisitor_checkUpdate) {
    std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
    std::shared_ptr<ApplyRessourceSharingDomainIdVisitor> rsd_v = std::make_shared<ApplyRessourceSharingDomainIdVisitor>();
    RegularDcdMapValuePrinter p(mapFull);
    mapFull->setOwnerCell(GridCellID(3, 3));
    setSimTime(1.4);
    ymf_v->setTime(simTime());
    rsd_v->setTime(simTime());
    EXPECT_EQ(rsd_v->getTime(), 1.4) << "Time in rsd-visitor not set correctly";
    EXPECT_EQ(rsd_v->getLastCallTime(), 0.0) << "Last call time must be zero at start.";
    mapFull->computeValues(ymf_v);
    mapFull->visitCells(*rsd_v);
    EXPECT_EQ(rsd_v->getLastCallTime(), 1.4) << "Last call time was not updated correctly.";

    EXPECT_EQ(mapFull->getCell(GridCellID(1,1)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(3,3)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(4,4)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(6,3)).val()->getResourceSharingDomainId(), otherRessourceSharingDomainId);

    //update with old value  but with ownRessourceSharingDomainId
    update(mapFull, 6, 3, 3, 1, 1, ownRessourceSharingDomainId);
    //not time update
    setSimTime(2.4);
    ymf_v->setTime(simTime());
    rsd_v->setTime(simTime());

    mapFull->computeValues(ymf_v);
    EXPECT_EQ(rsd_v->getTime(), 2.4);
    EXPECT_EQ(rsd_v->getLastCallTime(), 1.4);
    mapFull->visitCells(*rsd_v);
    EXPECT_EQ(rsd_v->getLastCallTime(), 2.4);
    EXPECT_EQ(mapFull->getCell(GridCellID(1,1)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(3,3)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(4,4)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(6,3)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId) << "wrong RSD cell";


    //with time update

}


TEST_F(DpmmResourceSharingIdTest, RsdNeighborhoodCountVisitor_NeverRun) {
    std::shared_ptr<RsdNeighborhoodCountVisitor> rsdCount_v = std::make_shared<RsdNeighborhoodCountVisitor>();
    EXPECT_EQ(rsdCount_v->getCount(), 0);

}

TEST_F(DpmmResourceSharingIdTest, RsdNeighborhoodCountVisitor_RunWithNoRsdId) {
    std::shared_ptr<RsdNeighborhoodCountVisitor> rsdCount_v = std::make_shared<RsdNeighborhoodCountVisitor>();
    setSimTime(1.0);

    // in case of no RSD info (rsdid <0) return ego count of 1
    rsdCount_v->reset(simTime(), -1); // -1: no rsdid
    mapFull->visitCells(*rsdCount_v);
    EXPECT_EQ(rsdCount_v->getCount(), 1);

}

TEST_F(DpmmResourceSharingIdTest, RsdNeighborhoodCountVisitor_ValidCount) {
    std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
    std::shared_ptr<ApplyRessourceSharingDomainIdVisitor> rsd_v = std::make_shared<ApplyRessourceSharingDomainIdVisitor>();
    std::shared_ptr<RsdNeighborhoodCountVisitor> rsdCount_v = std::make_shared<RsdNeighborhoodCountVisitor>();
    std::stringstream out;

    RegularDcdMapValuePrinter p(mapFull);
    mapFull->setOwnerCell(GridCellID(3, 3));
    setSimTime(1.4);
    ymf_v->setTime(simTime());
    rsd_v->setTime(simTime());
    mapFull->computeValues(ymf_v);
    mapFull->visitCells(*rsd_v);

    // count all cells where the local RSD is present
    rsdCount_v->reset(simTime(), ownRessourceSharingDomainId);
    mapFull->visitCells(*rsdCount_v);
    EXPECT_EQ(rsdCount_v->getCount(), 9);


    //update with old value  but with ownRessourceSharingDomainId
    setSimTime(2.4);
    ymf_v->setTime(simTime());
    rsd_v->setTime(simTime());
    rsdCount_v->reset(simTime(), ownRessourceSharingDomainId);
    update(mapFull, 6, 3, 3, 1, 1, ownRessourceSharingDomainId);
    mapFull->computeValues(ymf_v); // still count 5
    mapFull->visitCells(*rsd_v); // but this time it counts towards the rsd_count
    mapFull->visitCells(*rsdCount_v);
    EXPECT_EQ(rsdCount_v->getCount(), 9+5);

}

