/*
 * IPositionEmulator.h
 *
 *  Created on: Jun 22, 2021
 *      Author: mru
 */
#pragma once

#include <omnetpp.h>
#include <artery/utility/Geometry.h>
#include "crownet/common/util/crownet_util.h"
#include "crownet/crownet.h"

namespace crownet {
    class IPositionEmulator {
    public:
        virtual ~IPositionEmulator() = default;

        virtual void useEmulatedPositionSource(bool) = 0;
        virtual bool usesEmulatedPositionSource() = 0;

        virtual void injectEmulatedPosition(inet::Coord pos, artery::Angle heading, double speed) = 0;
    };
}
