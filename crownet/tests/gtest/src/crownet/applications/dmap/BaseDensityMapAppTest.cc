/*
 * BaseDensityMapAppTest.cc
 *
 *  Created on: Aug 24, 2023
 *      Author: vm-sts
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "main_test.h"
#include "crownet/crownet_testutil.h"


#include "crownet/applications/dmap/BaseDensityMapApp.h"
#include "crownet/dcd/generic/transmissionOrderStrategies/InsertionOrderedCellIdStream.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"


using namespace crownet;



class BaseDensityMapAppMock: public BaseDensityMapApp {
public:
    MOCK_METHOD(void, initialize, (int stage), (override));
    MOCK_METHOD(const inet::b, getMaxPdu, (), (const, override));

    void setDcdMap(std::shared_ptr<RegularDcdMap> p){dcdMap =p;}
    BurstInfo getBurstInfoPub(inet::b b) const {return getBurstInfo(b); }


};

class CellIdStreamMock: public InsertionOrderedCellIdStream<GridCellID, IntIdentifer, omnetpp::simtime_t> {
public:
    MOCK_METHOD(const int, size, (const omnetpp::simtime_t& now), (const, override));
};


class BaseDensityMapAppMockTest: public MapTest {
public:
    BaseDensityMapAppMockTest(): MapTest() {}


};


