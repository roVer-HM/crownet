/*
 * CellEntryTest.cc
 *
 *  Created on: Oct 8, 2020
 *      Author: sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "crownet/crownet_testutil.h"
#include "crownet/dcd/regularGrid/RegularCell.h"

using namespace crownet;



class RegularCellComparisonTest : public CellTest {
 public:
  RegularCellComparisonTest() : CellTest(){}
  void SetUp() override {
      cella_44 = RegularCell(dcdFactory->getTimeProvider(), GridCellID(3, 4), IntIdentifer(44));
      cella_55 = RegularCell(dcdFactory->getTimeProvider(), GridCellID(3, 4), IntIdentifer(55));
      cellb_44 = RegularCell(dcdFactory->getTimeProvider(), GridCellID(3, 7), IntIdentifer(44));
      cellc_44 = RegularCell(dcdFactory->getTimeProvider(), GridCellID(4, 0), IntIdentifer(44));
      celld_44 = RegularCell(dcdFactory->getTimeProvider(), GridCellID(9, 9), IntIdentifer(44));
  }

 protected:
  // total order: cella_44 < cella_55 < cellb_44 < cellc_44, < celld_44
  RegularCell cella_44;
  RegularCell cella_55;
  RegularCell cellb_44;
  RegularCell cellc_44;
  RegularCell celld_44;
};


TEST_F(RegularCellComparisonTest, Test_constructor) {
  Cell<int, int, SimTime> cell{dcdFactory->getTimeProvider(), 12, 13};
  EXPECT_EQ(cell.getCellId(), 12);
  EXPECT_EQ(cell.getOwnerId(), 13);
}

TEST_F(RegularCellComparisonTest, Test_str) {
  Cell<int, int, SimTime> cell{dcdFactory->getTimeProvider(), 12, 13};
  std::string s = "{cell_id: 12 owner_id: 13, val:n/a}";
  EXPECT_STREQ(cell.str().c_str(), s.c_str());
}

TEST_F(RegularCellComparisonTest, equality_1) {
  EXPECT_TRUE(cella_44 == cella_44);
  EXPECT_TRUE(cella_55 == cella_55);
  EXPECT_TRUE(cellb_44 == cellb_44);
  EXPECT_TRUE(cellc_44 == cellc_44);
  EXPECT_TRUE(celld_44 == celld_44);
}

TEST_F(RegularCellComparisonTest, equality_2) {
  EXPECT_FALSE(cella_44 == cella_55);
  EXPECT_FALSE(cella_55 == cellb_44);
  EXPECT_FALSE(cellb_44 == cellc_44);
  EXPECT_FALSE(cellc_44 == celld_44);
  EXPECT_FALSE(celld_44 == cella_44);
}

TEST_F(RegularCellComparisonTest, less_than_1) {
  EXPECT_TRUE(cella_44 < cella_55);
  EXPECT_TRUE(cella_55 < cellb_44);
  EXPECT_TRUE(cellb_44 < cellc_44);
  EXPECT_TRUE(cellc_44 < celld_44);
  EXPECT_FALSE(celld_44 < cella_44);
}

