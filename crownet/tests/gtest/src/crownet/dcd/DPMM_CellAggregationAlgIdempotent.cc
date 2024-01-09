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
#include "crownet/crownet_testutil.h"

#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include <iostream>
#include <fstream>
#include <boost/predef/other/endian.h>
#include <boost/endian/conversion.hpp>
#include <boost/algorithm/hex.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/name_generator_sha1.hpp>
namespace fs = boost::filesystem;

using namespace crownet;


class DPMM_CellAggregationAlgIdempotentTest : public MapTest {
 public:
  using Entry = IEntry<IntIdentifer, omnetpp::simtime_t>;
  DPMM_CellAggregationAlgIdempotentTest(): MapTest(), cell(GridCellID{5,5}){}

  static const int nodeID = 100;

  void SetUp() override {
      mapFull = dcdFactory->create_shared_ptr(3);
      clearMap();
  }

  void clearMap() {
      clear_map(mapFull, 0.0);
  }

  void last_call_at(CellAggregationAlgorihm<RegularCell> *v, simtime_t t){
      EXPECT_EQ(t.dbl(), mapFull->getLastComputedAt().dbl());
  }


  void map_comupte_check_multi_call(CellAggregationAlgorihm<RegularCell> *v){

      //1. First call with current time larger than lastCallTime.
      update(mapFull, cell, DPMM_CellAggregationAlgIdempotentTest::nodeID, 5, simTime() , 100);

      last_call_at(v, simtime_t::ZERO);    // last call time not set
      v->setTime(simTime());
      mapFull->computeValues(v); // must update last call time
      last_call_at(v, simTime()); // last call time must be updated here

      // cell contains only one vale thus selected value must be equal to that value but a copy!.
      auto selected_value = mapFull->getCell(cell).val();
      auto entry = mapFull->getCell(cell).getData()[nodeID];

      expect_different_object_but_same_content(entry, selected_value);

      incrementSimTime(1.0); // simtime not null now!
      v->setTime(simTime());
      mapFull->computeValues(v); // must update last call time
      expect_different_object_but_same_content(entry, selected_value);


      //2. Second call at the same time does not execute visitor
      // change value and recall computeValues without updating
      // the simulation time to simulate multiple calls at the same time.
      // the selected value must not change in this case.
      entry->incrementCount(simTime(), simTime(), 1.0); //increment value
      last_call_at(v, simTime());    // last call time not changed
      v->setTime(simTime());
      mapFull->computeValues(v); // Idempotent call, nothing must change
      last_call_at(v, simTime()); // last call time must be updated here

      // count of selected value is smaller (by one) as the set value due to idempotent call.
      EXPECT_EQ(mapFull->getCell(cell).val()->getCount() + 1, mapFull->getCell(cell).getData()[nodeID]->getCount());

      //3. Third call with time larger time will trigger call again.
      simtime_t old_time = incrementSimTime(5.0);
      last_call_at(v, old_time);    // last call time not changed
      v->setTime(simTime());
      mapFull->computeValues(v); // Idempotent call, nothing must change
      last_call_at(v, simTime()); // last call time must be updated here

      // cell contains only one vale thus selected value must be equal to that value but a copy!.
      selected_value = mapFull->getCell(cell).val();
      entry = mapFull->getCell(cell).getData()[nodeID];

      expect_different_object_but_same_content(entry, selected_value);

  }

 protected:
  std::shared_ptr<RegularDcdMap> mapFull;
  GridCellID cell{5, 5};

};

TEST_F(DPMM_CellAggregationAlgIdempotentTest, YmfVisitor_computeValue_idempotent1) {

    setSimTime(20.0);
    YmfVisitor v {simTime()};
    map_comupte_check_multi_call(&v);
}

TEST_F(DPMM_CellAggregationAlgIdempotentTest, YmfPlusDistVisitor_computeValue_idempotent1) {

    setSimTime(20.0);
    YmfPlusDistVisitor v {0.5, simTime()}; // alpha , time
    map_comupte_check_multi_call(&v);

}

TEST_F(DPMM_CellAggregationAlgIdempotentTest, YmfPlusDistStepVisitor_computeValue_idempotent1) {

    setSimTime(20.0);
    YmfPlusDistStepVisitor v {0.95, simTime(), 80.0}; // alpha, time, dist
    map_comupte_check_multi_call(&v);

}

TEST_F(DPMM_CellAggregationAlgIdempotentTest, YmfVisitor_computeValue_idempotent_firstcall_zero) {

    setSimTime(0.0); //current time and default value of lastCallTime are equal.
                     //First call must still work as visitor checks for first call
    YmfVisitor v {simTime()};
    map_comupte_check_multi_call(&v);
}

TEST_F(DPMM_CellAggregationAlgIdempotentTest, YmfPlusDistVisitor_computeValue_idempotent_firstcall_zero) {

    setSimTime(0.0); //current time and default value of lastCallTime are equal.
                     //First call must still work as visitor checks for first call
    YmfPlusDistVisitor v {0.5, simTime()}; // alpha , time
    map_comupte_check_multi_call(&v);

}

TEST_F(DPMM_CellAggregationAlgIdempotentTest, YmfPlusDistStepVisitor_computeValue_idempotent_firstcall_zero) {

    setSimTime(0.0); //current time and default value of lastCallTime are equal.
                     //First call must still work as visitor checks for first call
    YmfPlusDistStepVisitor v {0.95, simTime(), 80.0}; // alpha, time, dist
    map_comupte_check_multi_call(&v);

}

