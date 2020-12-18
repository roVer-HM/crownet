/*
 * IdentifierTest.cc
 *
 *  Created on: Dec 1, 2020
 *      Author: sts
 */

#include <functional>
#include <memory>
#include <string>

#include "main_test.h"
#include "rover/dcd/identifier/Identifiers.h"

using namespace rover;

TEST(GridCellID_Test, operator_st_true) {
  EXPECT_TRUE(GridCellID(2, 4) < GridCellID(3, 4));
  EXPECT_TRUE(GridCellID(3, 4) < GridCellID(3, 5));
}

TEST(GridCellID_Test, operator_st_equal) {
  EXPECT_FALSE(GridCellID(2, 4) < GridCellID(2, 4));
}

TEST(GridCellID_Test, operator_st_false) {
  EXPECT_FALSE(GridCellID(3, 4) < GridCellID(2, 4));
  EXPECT_FALSE(GridCellID(3, 5) < GridCellID(3, 4));
}

TEST(GridCellID_Test, operator_eq) {
  EXPECT_TRUE(GridCellID(2, 4) == GridCellID(2, 4));
  EXPECT_FALSE(GridCellID(3, 4) == GridCellID(2, 4));
  EXPECT_FALSE(GridCellID(3, 5) == GridCellID(3, 4));
}

TEST(IntIdentifer_Test, operator_st_true) {
  EXPECT_TRUE(IntIdentifer(2) < IntIdentifer(3));
  EXPECT_TRUE(IntIdentifer(3) < IntIdentifer(5));
}

TEST(IntIdentifer_Test, operator_st_equal) {
  EXPECT_FALSE(IntIdentifer(2) < IntIdentifer(2));
}

TEST(IntIdentifer_Test, operator_st_false) {
  EXPECT_FALSE(IntIdentifer(4) < IntIdentifer(2));
  EXPECT_FALSE(IntIdentifer(5) < IntIdentifer(3));
}

TEST(IntIdentifer_Test, operator_eq) {
  EXPECT_TRUE(IntIdentifer(2) == IntIdentifer(2));
  EXPECT_FALSE(IntIdentifer(3) == IntIdentifer(4));
  EXPECT_FALSE(IntIdentifer(4) == IntIdentifer(3));
}
