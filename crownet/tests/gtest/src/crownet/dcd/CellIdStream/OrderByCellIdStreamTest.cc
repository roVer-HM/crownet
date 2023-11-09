/*
 * OrderByCellIdStreamTest.cc
 *
 *  Created on: Aug 28, 2023
 *      Author: vm-sts
 */




#include "main_test.h"
#include "crownet/crownet_testutil.h"

#include "crownet/dcd/generic/transmissionOrderStrategies/OrderdCellIdVectorProvider.h"
#include "crownet/dcd/generic/transmissionOrderStrategies/OrderByCellIdStream.h"


class OrderByCellIdStreamTestF : public MapTest {
public:
    // orderBySentLastAsc_DistAsc
    // orderBySentLastAsc_DistDesc
    using MapPtr = std::shared_ptr<RegularDcdMap>;
    using AlgPtr = std::shared_ptr<YmfVisitor>;

    // check ordering
    std::pair<MapPtr, AlgPtr> setupTest(std::string streamType){
        auto map = dcdFactory->create_shared_ptr(IntIdentifer(1), streamType);

         setup_full(map);
         setSimTime(35.0);
         auto now = simTime();
         AlgPtr v = std::make_shared<YmfVisitor>();
         map->setOwnerCell(GridCellID(3, 3));
         v->setTime(now);
         map->computeValues(v);
         return std::make_pair(map, std::move(v));
    }

    // same setup as for InsertionOrderedCellIdStreamTest
    std::pair<MapPtr, AlgPtr> setupTest2(std::string streamType){
        setSimTime(1.0);
        AlgPtr v = std::make_shared<YmfVisitor>();
        auto map = dcdFactory->create_shared_ptr(IntIdentifer(1), streamType);
        map->setOwnerCell(GridCellID{6,6});

        EXPECT_EQ(map->getCellKeyStream()->size(simTime()), 0);
        this->update(map, GridCellID{0, 0}, 1, 5, simTime().dbl()); // id, count, time
        this->update(map, GridCellID{1, 1}, 1, 10, simTime().dbl());
        this->update(map, GridCellID{2, 2}, 1, 10, simTime().dbl());
        setSimTime(10.0);
        // update of existing cell does not change order
        this->update(map, GridCellID{1, 1}, 1, 7, simTime().dbl()); // id, count, time
        v->setTime(simTime());
        map->computeValues(v);
        EXPECT_EQ(map->getCellKeyStream()->size(simTime()), 3) << "Test fixture setup check. If this fails check overall setupTest function";

        return std::make_pair(map, v);
    }

};


TEST_F(OrderByCellIdStreamTestF, vec_select_on_by_one_until_throw) {
    std::pair<MapPtr, AlgPtr> test = setupTest("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto v = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    EXPECT_EQ(stream->size(now), 4);
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(3,3));
    EXPECT_EQ(map->getCell(GridCellID(3,3)).lastSent(), now);

    EXPECT_EQ(stream->size(now), 3);
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(4,4));
    EXPECT_EQ(map->getCell(GridCellID(4,4)).lastSent(), now);

    EXPECT_EQ(stream->size(now), 2);
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(1,1));
    EXPECT_EQ(map->getCell(GridCellID(1,1)).lastSent(), now);

    EXPECT_EQ(stream->size(now), 1);
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(6,3));
    EXPECT_EQ(map->getCell(GridCellID(6,3)).lastSent(), now);

    EXPECT_EQ(stream->size(now), 0);
    EXPECT_FALSE(stream->hasNext(now));

    EXPECT_THROW({
        try {
            stream->nextCellId(now);
        } catch (const omnetpp::cRuntimeError& e) {
            EXPECT_STREQ("No valid nextCellId", e.what());
            throw;
        }
    },  omnetpp::cRuntimeError);
}

/**
 *
 */
TEST_F(OrderByCellIdStreamTestF, vec_ignore_invlaid_cells_and_all_after) {
    std::pair<MapPtr, AlgPtr> test = setupTest("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto v = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    // after computeValues all cells are valid
    EXPECT_EQ(stream->size(now), 4) << "after initial computeValues all cells should be valid";
    map->getCell(GridCellID(1,1)).getLocal()->reset();

    ResetCellVisitor  rv(now);
    GridCellID c(6,3);
    map->getCell(c).acceptSet(&rv); // reset cell after compute values
    EXPECT_FALSE(map->getCell(c).hasValid()) << "Cell [6,3] should be invalid.";
    EXPECT_TRUE(map->getCell(c).val() == nullptr) << "cell [6,3] should not have a value";

    EXPECT_EQ(stream->size(now), 3) << "should be 3, because the third element is invalid and the count/hasNext should jump over invalid cells.";
}

TEST_F(OrderByCellIdStreamTestF, vec_order_sentAsc_dist_Asc){
    /**
     * [1, 1] Local = 2 / Other = 4
     * [3, 3] Local = 3 / Other = 3    <--- own location here
     * [4, 4] Local = 2 / Other = 0
     * [6, 3] Local = 0 / Other = [5, 7, 9]
     * current time 35.0
     */
    std::pair<MapPtr, AlgPtr> test = setupTest("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto v = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    EXPECT_EQ(stream->size(now), 4)<< "before access there should be all cells present";
    std::vector<GridCellID> ordered_ids = stream->getNumCellsOrLess(now, 1000); // only 4
    EXPECT_EQ(stream->size(now), 0) << "extracting all cells should leave stream empty";
    EXPECT_EQ(ordered_ids.size(), 4) << "extracted cells should be all in vector";

    std::vector<GridCellID> expectedOrder = {GridCellID(3,3), GridCellID(4,4), GridCellID(1,1), GridCellID(6,3)};
    cmp_vec(ordered_ids, expectedOrder);

    // change order based on lastSent value.

    incrementSimTime(3);
    map->getCell(GridCellID(4,4)).sentAt(simTime()); // will change order of cells
    incrementSimTime(1);
    now = simTime();

    v->setTime(now);
    map->computeValues(v);

    EXPECT_EQ(stream->size(now), 4)<< "before access there should be all cells present";
    ordered_ids = stream->getNumCellsOrLess(now, 1000); // only 4
    EXPECT_EQ(stream->size(now), 0) << "extracting all cells should leave stream empty";
    EXPECT_EQ(ordered_ids.size(), 4) << "extracted cells should be all in vector";

    expectedOrder = {GridCellID(3,3),  GridCellID(1,1), GridCellID(6,3), GridCellID(4,4)};
    cmp_vec(ordered_ids, expectedOrder);

}

/* *************************************************************************************
 * Same Test as for InsertionOrderedCellIdStreamTest (Not using Parametric test due    *
 * different ordering make test results matching hard
 *
 *   Map owner for these test is at [6,6] with distance equivalent (triangle inequality)                  *
 *
 *      cells       distance equivalent (triangle inequality) to [6,6]
 *      [0,0]           6 + 6 = 12
 *      [1,1]           5 + 5 = 10
 *      [2,2]           4 + 4 = 8
 *
 * *************************************************************************************/

/**
 * InsertionOrderedCellIdStream cell order is defined by First Seen First Out.
 *  - The order is not changed when updating cells out of order.
 */
TEST_F(OrderByCellIdStreamTestF, orderOfCells) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    // stream will stop producing once all cells are returned for the same time
    EXPECT_EQ(stream->nextCellId(now), GridCellID(2, 2));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(1, 1));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_FALSE(stream->hasNext(now));



    incrementSimTime(5.0);
    now = simTime(); // now cells will be returned again in the same order
    stream->update(now);

    EXPECT_EQ(stream->nextCellId(now), GridCellID(2, 2));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(1, 1));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_FALSE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 0);


    // for a new time step cell will be available
    EXPECT_TRUE(stream->hasNext(now + 0.1));
    EXPECT_EQ(stream->size(now + 0.1), 3);

}

/**
 * Accessing more cells from the stream  will only return at most all valid cells
 *  or if all cells are valid it will return all cells in the queue.
 *  Case: all cells in queue are valid.
 */
TEST_F(OrderByCellIdStreamTestF, list_of_cell_ids_more) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 3); // there are only 3 valid items
    std::vector<GridCellID> expected_vec = {GridCellID(2, 2), GridCellID(1, 1), GridCellID(0, 0)};
    cmp_vec(cellIdList, expected_vec);


}

/**
 * Accessing more cells from the stream  will only return at most all valid cells
 *  or if all cells are valid it will return all cells in the queue
 *  Case: only 2 cells are valid.
 */
TEST_F(OrderByCellIdStreamTestF, list_of_cell_ids_more2) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);
    // will invalidate second element but stream will skip this and retun all valid
    map->getCell(GridCellID(1,1)).sentAt(now);

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 2); // there are only 3 valid items
    std::vector<GridCellID> expected_vec = {GridCellID(2, 2), GridCellID(0, 0)};
    cmp_vec(cellIdList, expected_vec);
    EXPECT_FALSE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 0);
}

TEST_F(OrderByCellIdStreamTestF, keep_order_if_not_all_cells_are_used_in_one_update_cycle) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);

    // use up 2 out of 3 cells
    auto cellIdList = stream->getNumCellsOrLess(now, 2);
    std::vector<GridCellID> expected_vec = {GridCellID(2, 2), GridCellID(1, 1)};
    cmp_vec(cellIdList, expected_vec);
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 1);

    incrementSimTime(2);
    now = simTime();
    ymf->setTime(now);
    map->computeValues(ymf);
    cellIdList = stream->getNumCellsOrLess(now, 100); // order will sort by  SentLast fist and then distance. Thus order will change
    expected_vec = {GridCellID(0, 0), GridCellID(2, 2), GridCellID(1, 1)};
    cmp_vec(cellIdList, expected_vec, "order of cells is now based on sentTime and then distance. Thus the last cell not sent in the last cycle is now at the front of line. See 'orderBySentLastAsc_DistAsc' stream type.");

    EXPECT_FALSE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 0);

}


TEST_F(OrderByCellIdStreamTestF, list_of_cell_ids_with_only_one_valid) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);
    stream->nextCellId(now);
    stream->nextCellId(now);
    EXPECT_EQ(stream->size(now), 1) << "invalidate 2 out of 3. That should leave one item behind";

    // access stream via getNumCellsOrLess
    auto cellIdList = stream->getNumCellsOrLess(now, 1);
    EXPECT_EQ(cellIdList.size(), 1) << "accessing one cell and one cell should be left";

}

TEST_F(OrderByCellIdStreamTestF, stream_size_if_one_cell_is_invalid) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3) << "all valid";
    ResetCellVisitor v{now};
    map->getCell(GridCellID(1, 1)).acceptSet(&v); // reset cell.

    EXPECT_EQ(stream->size(now), 2) << "expected count of 2 as the whole list is searched, skipping invalid cells.";
}


TEST_F(OrderByCellIdStreamTestF, list_of_cell_asking_for_fewer_cells) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 2);
    EXPECT_EQ(cellIdList.size(), 2); // there are only 3 valid items
    std::vector<GridCellID> expected_vec = {GridCellID(2, 2), GridCellID(1, 1)};
    cmp_vec(cellIdList, expected_vec);

    //new head at
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_EQ(stream->size(now), 0); // zero because list is consumed.
}

TEST_F(OrderByCellIdStreamTestF, returned_cells_must_have_sent_at_now) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);


    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(stream->nextCell(now).lastSent(), now) << "returned cell must have a set lastSent time at provided time";
    EXPECT_EQ(stream->nextCell(now).lastSent(), now) << "returned cell must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(GridCellID(0,0)).lastSent(), simtime_t::ZERO);

}

TEST_F(OrderByCellIdStreamTestF, returned_cells_must_have_sent_at_now_2) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);


    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(map->getCell(stream->nextCellId(now)).lastSent(), now) <<  "cell matching returned  cellId must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(stream->nextCellId(now)).lastSent(), now) <<  "cell matching returned  cellId must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(GridCellID(0,0)).lastSent(), simtime_t::ZERO);
}

TEST_F(OrderByCellIdStreamTestF, returned_cells_must_have_sent_at_now_3) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);

    auto id_list = stream->getNumCellsOrLess(now, 2);
    EXPECT_EQ(id_list.size(), 2);
    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(map->getCell(id_list[0]).lastSent(), now) <<  "cell matching returned cellId via getNumCellsOrLess must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(id_list[1]).lastSent(), now) <<  "cell matching returned cellId via getNumCellsOrLess must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(GridCellID(0,0)).lastSent(), simtime_t::ZERO);
}


TEST_F(OrderByCellIdStreamTestF, list_of_cell_ids_no_valid_cells) {
    std::pair<MapPtr, AlgPtr> test = setupTest2("orderBySentLastAsc_DistAsc");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);

    map->computeValues(ymf);
    EXPECT_EQ(stream->size(now), 3) << "after computeValues on map all cells in stream should be valid";

    ResetCellVisitor v{now};
    map->visitCells(&v); // invalidate all cells but do not call update on stream. Still, all cells must be seen as invalid.
    EXPECT_EQ(stream->size(now), 0) << "after invalidating map the stream must be empty!";
}


