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
    std::vector<GridCellID> vec_expected = {GridCellID{3, 3}, GridCellID{4, 4}, GridCellID{6, 3}, GridCellID{1, 1}};
    cmp_vec(vec, vec_expected);


    map->getCell(GridCellID(3,3)).sentAt(simTime());
    incrementSimTime(5.0);
    vec = order.getCellOrder(map.get());
    vec_expected = { GridCellID{4, 4}, GridCellID{6, 3}, GridCellID{1, 1}, GridCellID{3, 3}};
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
    std::vector<GridCellID> vec_expected = {GridCellID{1, 1}, GridCellID{6, 3}, GridCellID{4, 4}, GridCellID{3, 3}};
    cmp_vec(vec, vec_expected);


    map->getCell(GridCellID(6,3)).sentAt(simTime());
    incrementSimTime(5.0);
    vec = order.getCellOrder(map.get());
    vec_expected = {GridCellID{1, 1}, GridCellID{4, 4}, GridCellID{3, 3}, GridCellID{6, 3}};
    cmp_vec(vec, vec_expected);

}
