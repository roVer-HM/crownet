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


    void setupTest(std::shared_ptr<RegularDcdMap> map, std::shared_ptr<YmfVisitor> visitor){
        setSimTime(1.0);
        EXPECT_EQ(map->getCellKeyStream()->size(simTime()), 0);
        this->update(map, GridCellID{0, 0}, 1, 5, simTime().dbl()); // id, count, time
        this->update(map, GridCellID{1, 1}, 1, 10, simTime().dbl());
        this->update(map, GridCellID{2, 2}, 1, 10, simTime().dbl());
        setSimTime(10.0);
        // update of existing cell does not change order
        this->update(map, GridCellID{1, 1}, 1, 7, simTime().dbl()); // id, count, time
        visitor->setTime(simTime());
        map->computeValues(visitor);
        auto now = simTime();
        EXPECT_EQ(map->getCellKeyStream()->size(now), 3);
    }

    void setupTestOneCell(std::shared_ptr<RegularDcdMap> map, std::shared_ptr<YmfVisitor> visitor){
        setSimTime(1.0);
        EXPECT_EQ(map->getCellKeyStream()->size(simTime()), 0);
        this->update(map, GridCellID{0, 0}, 1, 5, simTime().dbl()); // id, count, time
        setSimTime(10.0);
        visitor->setTime(simTime());
        map->computeValues(visitor);
        auto now = simTime();
        EXPECT_EQ(map->getCellKeyStream()->size(now), 1);
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
 * InsertionOrderedCellIdStream cell order is defined by First Seen Last Out.
 *  - The order is not changed when updating cells out of order.
 *  - Cells are kept in a deque. Once the abort condition is met no new cells are provided.
 */
TEST_F(InsertionOrderedCellIdStreamTest, orderOfCells) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    // stream will stop producing once all cells are returned for the same time
    EXPECT_EQ(stream->nextCellId(now), GridCellID(2, 2));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(1, 1));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_FALSE(stream->hasNext(now));

    incrementSimTime(5.0);
    now = simTime(); // now cells will be returned again in the same order
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
 *  or if all cells are valid ist will return all cells in the queue.
 *  Case: all cells in queue are valid.
 */
TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_more) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 3); // there are only 3 valid items
    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(cellIdList[0], GridCellID(2, 2));
    EXPECT_EQ(cellIdList[1], GridCellID(1, 1));
    EXPECT_EQ(cellIdList[2], GridCellID(0, 0));

}

/**
 * Accessing more cells from the stream  will only return at most all valid cells
 *  or if all cells are valid ist will return all cells in the queue
 *  Case: only 2 cells are valid.
 */
TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_more2) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3);
    map->getCell(GridCellID(1,1)).sentAt(now); // will invalidate second element thus only 1 cell will be return

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 1); // there are only 3 valid items
    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(cellIdList[0], GridCellID(2, 2));
    EXPECT_FALSE(stream->hasNext(now));

}

TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_with_only_one_valid) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3);
    stream->nextCellId(now);
    stream->nextCellId(now);
    EXPECT_EQ(stream->size(now), 1) << "invalidate 2 out of 3. Asume that one is still left";

    // access stream via getNumCellsOrLess
    auto cellIdList = stream->getNumCellsOrLess(now, 1);
    EXPECT_EQ(cellIdList.size(), 1) << "accessing one cell and one cell should be left";

}

TEST_F(InsertionOrderedCellIdStreamTest, stream_size_if_one_cell_is_invalid) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3) << "all valid";

    map->getCell(GridCellID(1, 1)).get(IntIdentifer(1))->reset();

    EXPECT_EQ(stream->size(now), 1) << "expected count of 1 as the second item does not have a valid measure";

}


TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_less) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 2);
    EXPECT_EQ(cellIdList.size(), 2); // there are only 3 valid items
    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(cellIdList[0], GridCellID(2, 2));
    EXPECT_EQ(cellIdList[1], GridCellID(1, 1));

    //new head at
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
    EXPECT_EQ(stream->size(now), 0); // zero because list is consumed.
}

TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_less_with_sent) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3);

    auto cellIdList = stream->getNumCellsOrLess(now, 2);
    EXPECT_EQ(cellIdList.size(), 2); // there are only 3 valid items
    // stream is a circle. Thus will start from beginning if cells are not reset
    EXPECT_EQ(cellIdList[0], GridCellID(2, 2));
    EXPECT_EQ(cellIdList[1], GridCellID(1, 1));
    for (auto& c : cellIdList){
        map->getCell(c).sentAt(now);
    }

    //new head at
    EXPECT_TRUE(stream->hasNext(now));
    EXPECT_EQ(stream->size(now), 1) << "only one left in queue";
    EXPECT_EQ(stream->nextCellId(now), GridCellID(0, 0));
}

TEST_F(InsertionOrderedCellIdStreamTest, list_of_cell_ids_no_valid_cells) {
    std::shared_ptr<RegularDcdMap> map = this->dcdFactory->create_shared_ptr(1, "insertionOrder");
    auto stream = map->getCellKeyStream();

    auto ymf = std::make_shared<YmfVisitor>();

    this->setupTest(map, ymf);
    auto now = simTime();
    EXPECT_EQ(stream->size(now), 3);
    // invalidate first element
    map->getCell(GridCellID(2,2)).sentAt(now);
    EXPECT_EQ(stream->size(now), 0);

    auto cellIdList = stream->getNumCellsOrLess(now, 100);
    EXPECT_EQ(cellIdList.size(), 0); // no valid cells
    EXPECT_FALSE(stream->hasNext(now));


}
