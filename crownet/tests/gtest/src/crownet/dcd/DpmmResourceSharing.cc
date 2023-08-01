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


using namespace crownet;

class DpmmResourceSharingIdTest : public MapTest{
    /**
     * By default all other data present is from nodes in other resource sharing domains.
     */
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DpmmResourceSharingIdTest()
      : MapTest() {
      mapFull = dcdFactory->create_shared_ptr(3);
  }

  void SetUp() override {
    setSimTime(0.0);

    setup_full(mapFull);

    entry(mapFull, c1_1_800)->setResourceSharingDomainId(otherRessourceSharingDomainId);
    entry(mapFull, c4_4_801)->setResourceSharingDomainId(otherRessourceSharingDomainId);
    entry(mapFull, c6_3_803)->setResourceSharingDomainId(otherRessourceSharingDomainId);
    entry(mapFull, c6_3_805)->setResourceSharingDomainId(otherRessourceSharingDomainId);
    entry(mapFull, c6_3_808)->setResourceSharingDomainId(otherRessourceSharingDomainId);

  }

 protected:
  std::shared_ptr<RegularDcdMap> mapFull;
};




TEST_F(DpmmResourceSharingIdTest, ApplyRessourceSharingDomainIdVisitor_checkUpdate) {
    std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
    std::shared_ptr<ApplyRessourceSharingDomainIdVisitor> rsd_v = std::make_shared<ApplyRessourceSharingDomainIdVisitor>();

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
    entry(mapFull, c6_3_803)->setResourceSharingDomainId(ownRessourceSharingDomainId);
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


TEST_F(DpmmResourceSharingIdTest, InfectuouseRessourceSharingDomainValid) {

    /*
     * For cells without any local measurement the RSD for the selected value is determined based on the
     * RSD provided in the visitor. If any valid measurement, even if not selected, has the same RSD.
     *
     */

    std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
    std::shared_ptr<ApplyRessourceSharingDomainIdVisitor> rsd_v = std::make_shared<ApplyRessourceSharingDomainIdVisitor>();

    mapFull->setOwnerCell(GridCellID(3, 3));
    setSimTime(35.0);
    ymf_v->setTime(simTime());
    rsd_v->reset(simTime(), ownRessourceSharingDomainId);

    entry(mapFull, c6_3_808)->setResourceSharingDomainId(ownRessourceSharingDomainId); //set an old value to current RSD

    mapFull->computeValues(ymf_v);
    mapFull->visitCells(*rsd_v);

    EXPECT_EQ(mapFull->getCell(GridCellID(1,1)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(3,3)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(4,4)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(6,3)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId) << "Map Structure" << endl << mapFull->strFull();

    EXPECT_EQ(mapFull->getCell(GridCellID(6,3)).val()->getCount(), 5);
    EXPECT_EQ(entry(mapFull, c6_3_803)->getCount(), 5);


}

TEST_F(DpmmResourceSharingIdTest, InfectuouseRessourceSharingDomainInvalid) {

    /*
     * For cells without any local measurement the RSD for the selected value is determined based on the
     * RSD provided in the visitor. If any valid measurement, even if not selected, has the same RSD
     *
     * --> Do not use ownRessourceSharingDomainId of invalid cells.
     *
     */

    std::shared_ptr<YmfVisitor> ymf_v = std::make_shared<YmfVisitor>();
    std::shared_ptr<ApplyRessourceSharingDomainIdVisitor> rsd_v = std::make_shared<ApplyRessourceSharingDomainIdVisitor>();

    mapFull->setOwnerCell(GridCellID(3, 3));
    setSimTime(35.0);
    ymf_v->setTime(simTime());
    rsd_v->reset(simTime(), ownRessourceSharingDomainId);

    entry(mapFull, c6_3_808)->setResourceSharingDomainId(ownRessourceSharingDomainId); //set an old value to current RSD
    entry(mapFull, c6_3_808)->reset(); // but not valid!

    mapFull->computeValues(ymf_v);
    mapFull->visitCells(*rsd_v);

    EXPECT_EQ(mapFull->getCell(GridCellID(1,1)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(3,3)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(4,4)).val()->getResourceSharingDomainId(), ownRessourceSharingDomainId);
    EXPECT_EQ(mapFull->getCell(GridCellID(6,3)).val()->getResourceSharingDomainId(), otherRessourceSharingDomainId) << "Map Structure" << endl << mapFull->strFull();

    EXPECT_EQ(mapFull->getCell(GridCellID(6,3)).val()->getCount(), 5);
    EXPECT_EQ(entry(mapFull, c6_3_803)->getCount(), 5);


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

//    update(mapFull, 6, 3, 3, 1, 1, ownRessourceSharingDomainId);
    entry(mapFull, c6_3_803)->setResourceSharingDomainId(ownRessourceSharingDomainId);

    mapFull->computeValues(ymf_v); // still count 5
    mapFull->visitCells(*rsd_v); // but this time it counts towards the rsd_count
    mapFull->visitCells(*rsdCount_v);
    EXPECT_EQ(rsdCount_v->getCount(), 9+5);

}

