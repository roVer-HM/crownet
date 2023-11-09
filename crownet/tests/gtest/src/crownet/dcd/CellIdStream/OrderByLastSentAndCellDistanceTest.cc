/*
 * CellOrderTest.cc
 *
 *  Created on: Aug 28, 2023
 *      Author: vm-sts
 */


#include <functional>
#include <sstream>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"


#include "crownet/dcd/generic/transmissionOrderStrategies/InsertionOrderedCellIdStream.h"
#include "crownet/dcd/generic/transmissionOrderStrategies/OrderdCellIdVectorProvider.h"



using namespace crownet;




class CellOrderTestF : public MapTest {

};


TEST_F(CellOrderTestF, lastSentAsc_distanceAscDesc_compare){
    /*
     * lastSent is equal for all, sort will be defined only by distance. The reversed sorting order
     * of Desc must match Asc order.
     */
    auto map = dcdFactory->create_shared_ptr(IntIdentifer(1));

    setSimTime(1.0);
    for (int x = 0; x < 5; x++){
        for (int y = 0; y < 5; y++){
            // all cells same age
            incr(map, GridCellID(x, y), 1, 1.0);
        }
    }
    //last sent is 0.0
    setSimTime(5.0);
    YmfVisitor v{};
    map->computeValues(&v);
    map->setOwnerCell(GridCellID(0, 0));

    OrderByLastSentAndCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t> order_asc_asc(true, true);
    std::vector<GridCellID> vec_asc_asc = order_asc_asc.getCellOrder(map.get());


    OrderByLastSentAndCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t> order_asc_desc(true, false);
    std::vector<GridCellID> vec_asc_desc = order_asc_asc.getCellOrder(map.get());

    std::vector<GridCellID>  vec_asc_desc_reverse = vec_asc_desc;
    std::reverse(vec_asc_desc.begin(), vec_asc_desc.end());

    cmp_vec(vec_asc_asc, vec_asc_desc_reverse);

}


TEST_F(CellOrderTestF, lastSentAsc_distanceAsc){
    auto map = dcdFactory->create_shared_ptr(IntIdentifer(1));

    /**
     * [1, 1] Local = 2 / Other = 4
     * [3, 3] Local = 3 / Other = 3
     * [4, 4] Local = 2 / Other = 0
     * [6, 3] Local = 0 / Other = [5, 7, 9]
     */
    setup_full(map);
    setSimTime(35.0);
    YmfVisitor v{};
    map->computeValues(&v);
    map->setOwnerCell(GridCellID(3, 3));


    OrderByLastSentAndCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t> order(true, true);

    std::vector<GridCellID> vec = order.getCellOrder(map.get());
    std::vector<GridCellID> vec_expected = {GridCellID{3, 3}, GridCellID{4, 4}, GridCellID{1, 1}, GridCellID{6, 3}};
    cmp_vec(vec, vec_expected);


    map->getCell(GridCellID(3,3)).sentAt(simTime());
    incrementSimTime(5.0);
    vec = order.getCellOrder(map.get());
    vec_expected = { GridCellID{4, 4},  GridCellID{1, 1}, GridCellID{6, 3}, GridCellID{3, 3}};
    cmp_vec(vec, vec_expected);

}

TEST_F(CellOrderTestF, lastSentDesc_distanceAsc){
    auto map = dcdFactory->create_shared_ptr(IntIdentifer(1));

    /**
     * [1, 1] Local = 2 / Other = 4
     * [3, 3] Local = 3 / Other = 3
     * [4, 4] Local = 2 / Other = 0
     * [6, 3] Local = 0 / Other = [5, 7, 9]
     */
    setup_full(map);
    setSimTime(35.0);
    YmfVisitor v{};
    map->computeValues(&v);
    map->setOwnerCell(GridCellID(3, 3));


    OrderByLastSentAndCellDistance<GridCellID, IntIdentifer, omnetpp::simtime_t> order(true, false); // distance Desc.

    std::vector<GridCellID> vec = order.getCellOrder(map.get());
    std::vector<GridCellID> vec_expected = { GridCellID{6, 3}, GridCellID{1, 1}, GridCellID{4, 4}, GridCellID{3, 3}};
    cmp_vec(vec, vec_expected);


    map->getCell(GridCellID(6,3)).sentAt(simTime());
    incrementSimTime(5.0);
    vec = order.getCellOrder(map.get());
    vec_expected = {GridCellID{1, 1}, GridCellID{4, 4}, GridCellID{3, 3}, GridCellID{6, 3}};
    cmp_vec(vec, vec_expected);

}
