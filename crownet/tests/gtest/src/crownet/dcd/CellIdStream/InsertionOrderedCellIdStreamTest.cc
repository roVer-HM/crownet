/*
 * InsertionOrderedCellIdStream.cc
 *
 *  Created on: Aug 23, 2023
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

#include "crownet/common/Entry.h"
#include "crownet/dcd/regularGrid/RegularCell.h"
#include "crownet/dcd/regularGrid/RegularCellVisitors.h"
#include "crownet/dcd/regularGrid/RegularDcdMap.h"
#include "crownet/dcd/regularGrid/MapCellAggregationAlgorithms.h"
#include "crownet/dcd/generic/transmissionOrderStrategies/InsertionOrderedCellIdStream.h"


using namespace crownet;


class InsertionOrderedCellIdStreamTest: public MapTest {
public:
    InsertionOrderedCellIdStreamTest(): MapTest() {}

    using MapPtr = std::shared_ptr<RegularDcdMap>;
    using AlgPtr = std::shared_ptr<YmfVisitor>;

    std::pair<MapPtr, AlgPtr> setupTest(std::string streamType){
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


/**
 * InsertionOrderedCellIdStream provides only valid cells which have computed values.
 */
TEST_F(InsertionOrderedCellIdStreamTest, addCells) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    setSimTime(1.0);
    EXPECT_EQ(stream->size(simTime()), 0);
    this->update(map, GridCellID{0, 0}, 1, 5, simTime().dbl()); // id, count, time
    this->update(map, GridCellID{1, 1}, 1, 10, simTime().dbl());
    ymf->setTime(simTime());
    map->computeValues(ymf);
    EXPECT_EQ(stream->size(simTime()), 2);

}

/**
 * InsertionOrderedCellIdStream cell order is defined by First Seen First Out.
 *  - The order is not changed when updating cells out of order.
 */
TEST_F(InsertionOrderedCellIdStreamTest, orderOfCells) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    // stream will stop producing once all cells are returned for the same time
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(1, 1));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(2, 2));
    EXPECT_FALSE(stream->hasNext(now));



    incrementSimTime(5.0);
    now = simTime(); // now cells will be returned again in the same order
    stream->update(now);

    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(1, 1));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(2, 2));
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
TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_more) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();

    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 3); // there are only 3 valid items
    std::vector<GridCellID> expected_vec = {GridCellID(0, 0), GridCellID(1, 1), GridCellID(2, 2)};
    cmp_vec(cellIdList, expected_vec);


}

/**
 * Accessing more cells from the stream  will only return at most all valid cells
 *  or if all cells are valid it will return all cells in the queue
 *  Case: only 2 cells are valid.
 */
TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_more2) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);
    // will invalidate second element but stream will skip this and retun all valid
    map->getCell(GridCellID(1,1)).sentAt(now);

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 2); // there are only 3 valid items
    std::vector<GridCellID> expected_vec = {GridCellID(0, 0), GridCellID(2, 2)};
    cmp_vec(cellIdList, expected_vec);
    EXPECT_FALSE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 0);
}

TEST_F(InsertionOrderedCellIdStreamTest, keep_order_if_not_all_cells_are_used_in_one_update_cycle) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);

    // will invalidate one item. Will therefore return 2 (head is now at last cell)
    map->getCell(GridCellID(2,2)).sentAt(now);
    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    std::vector<GridCellID> expected_vec = {GridCellID(0, 0), GridCellID(1, 1)};
    cmp_vec(cellIdList, expected_vec);
    EXPECT_FALSE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 0);

    incrementSimTime(2);
    now = simTime();
    ymf->setTime(now);
    map->computeValues(ymf);
    cellIdList = stream->getNumCellsOrLess(now, 100); // assume 3 elements in order 3,0,1
    expected_vec = {GridCellID(2, 2), GridCellID(0, 0), GridCellID(1, 1)};
    cmp_vec(cellIdList, expected_vec, "list should start with last element of last computValues cycle as the head of the stream is not reset during update when 'insertionOrder' stream type is used.");
    EXPECT_FALSE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 0);

}


TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_with_only_one_valid) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
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

TEST_F(InsertionOrderedCellIdStreamTest, stream_size_if_one_cell_is_invalid) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3) << "all valid";
    ResetCellVisitor v{now};
    map->getCell(GridCellID(1, 1)).acceptSet(&v); // reset cell.

    EXPECT_EQ(stream->size(now), 2) << "expected count of 2 as the whole list is searched, skipping invalid cells.";
}


TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_asking_for_fewer_cells) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 2);
    EXPECT_EQ(cellIdList.size(), 2); // there are only 3 valid items
    std::vector<GridCellID> expected_vec = {GridCellID(0, 0), GridCellID(1, 1)};
    cmp_vec(cellIdList, expected_vec);

    //new head at
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(2, 2));
    EXPECT_EQ(stream->size(now), 0); // zero because list is consumed.
}

TEST_F(InsertionOrderedCellIdStreamTest, returned_cells_must_have_sent_at_now) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);


    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(stream->nextCell(now).lastSent(), now) << "returned cell must have a set lastSent time at provided time";
    EXPECT_EQ(stream->nextCell(now).lastSent(), now) << "returned cell must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(GridCellID(2,2)).lastSent(), simtime_t::ZERO);

}

TEST_F(InsertionOrderedCellIdStreamTest, returned_cells_must_have_sent_at_now_2) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
    auto map = test.first;
    auto ymf = test.second;
    auto stream = map->getCellKeyStream();
    auto now = simTime();


    EXPECT_EQ(stream->size(now), 3);


    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(map->getCell(stream->nextCellId(now)).lastSent(), now) <<  "cell matching returned  cellId must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(stream->nextCellId(now)).lastSent(), now) <<  "cell matching returned  cellId must have a set lastSent time at provided time";
    EXPECT_EQ(map->getCell(GridCellID(2,2)).lastSent(), simtime_t::ZERO);
}

TEST_F(InsertionOrderedCellIdStreamTest, returned_cells_must_have_sent_at_now_3) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
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
    EXPECT_EQ(map->getCell(GridCellID(2,2)).lastSent(), simtime_t::ZERO);
}


TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_no_valid_cells) {
    std::pair<MapPtr, AlgPtr> test = setupTest("insertionOrder");
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
