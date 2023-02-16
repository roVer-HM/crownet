/*
 * BurstIdSetTest.cc
 *
 *  Created on: Feb 15, 2023
 *      Author: vm-sts
 */


#include <gtest/gtest.h>
#include <omnetpp.h>
#include <memory>
#include <string>

#include "crownet/common/BurstIdSet.h"

namespace crownet {

TEST(BurstIdSet, AddMoreThenSize) {
    int setSize = 15;
    BurstIdSet s = BurstIdSet(setSize);
    for (int i = 0; i < 20; i++){
        simtime_t t = (double)(10.0+i);
        s.add(t);
    }
    ASSERT_EQ(s.size(), setSize);


    ASSERT_EQ(s.getSmallestValue().dbl(), 15.0);
    ASSERT_EQ(s.getLargestValue().dbl(), 29.0);


    for (int i=0; i < 5; i++){
        simtime_t t = (double)(10.0+i);
        ASSERT_FALSE(s.contains(t));
    }

    for (int i=5; i < 20; i++){
        simtime_t t = (double)(10.0+i);
        ASSERT_TRUE(s.contains(t));
    }
}

} // namesapce

