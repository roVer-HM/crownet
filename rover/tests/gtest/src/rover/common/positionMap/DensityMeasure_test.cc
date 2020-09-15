/*
 * grid_test.cc
 *
 *  Created on: Sep 11, 2020
 *      Author: sts
 */

#include "rover/common/positionMap/DensityMeasure.h"
#include <gtest/gtest.h>
#include <string>

TEST(LocalDensityMeasure, rest) {
  LocalDensityMeasure<int> m{};
  ASSERT_FALSE(m.valid());
  m.incrementCount(4.9);
  m.nodeIds.insert(32);
  ASSERT_TRUE(m.valid());
  m.reset();
  ASSERT_EQ(0, m.nodeIds.size());
  ASSERT_FALSE(m.valid());
}
