/*
 * BonnMotionMobilityClient.h
 *
 *  Created on: Jun 27, 2022
 *      Author: vm-sts
 */

#pragma once

#include "inet/mobility/base/LineSegmentsMobilityBase.h"
#include "inet/mobility/single/BonnMotionMobility.h"

using namespace inet;

namespace crownet {

class BonnMotionMobilityClient : public  inet::BonnMotionMobility {
public:
    BonnMotionMobilityClient();
    virtual ~BonnMotionMobilityClient();

    virtual void initTrace(const BonnMotionFile::Line* line, bool is3D, int bonnMotionLineNumner);

protected:

    /** @brief Initializes mobility model parameters. */
    virtual void initialize(int stage) override;

    virtual void initializePosition() override;

    /** @brief Initializes the position according to the mobility model. */
    virtual void setInitialPosition() override;

    /** @brief Overridden from LineSegmentsMobilityBase. */
    virtual void setTargetPosition() override;


    int bonnMotionLineNumner;

};

} /* namespace crownet */

