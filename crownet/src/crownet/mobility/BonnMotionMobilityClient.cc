/*
 * BonnMotionMobilityClient.cc
 *
 *  Created on: Jun 27, 2022
 *      Author: vm-sts
 */

#include "crownet/mobility/BonnMotionMobilityClient.h"
#include "crownet/mobility/BonnMotionMobilityServer.h"

namespace crownet {

Define_Module(BonnMotionMobilityClient);


BonnMotionMobilityClient::BonnMotionMobilityClient() {
    // TODO Auto-generated constructor stub

}

BonnMotionMobilityClient::~BonnMotionMobilityClient() {
    // TODO Auto-generated destructor stub
}

void BonnMotionMobilityClient::initTrace(const inet::BonnMotionFile::Line* line, bool is3D, int bonnMotionLineNumner) {
    Enter_Method_Silent();
    this->lines = line;
    this->is3D = is3D;
    this->bonnMotionLineNumner = bonnMotionLineNumner;
    currentLine = 0;
    computeMaxSpeed();
    EV_TRACE << "initializing BonnMotionMobilityClient done" << endl;
}

void BonnMotionMobilityClient::initialize(int stage)
{
    LineSegmentsMobilityBase::initialize(stage);

    EV_TRACE << "initializing BonnMotionMobilityClient stage " << stage << endl;
    if (stage == INITSTAGE_LOCAL) {
        // do nothing, will be managed by initTrace through direct method call from Server.
    }
}

void BonnMotionMobilityClient::initializePosition() {
    MobilityBase::initializePosition();
    lastUpdate = simTime();
    scheduleUpdate();
}

/** @brief Initializes the position according to the mobility model. */
void BonnMotionMobilityClient::setInitialPosition(){
    const BonnMotionFile::Line& vec = *lines;
    if (currentLine + (is3D ? 3 : 2) >= (int)vec.size()) {
            nextChange = -1;
            stationary = true;
            targetPosition = lastPosition;
            emit(BonnMotionMobilityServer::bonnMotionTargetReached, (double)bonnMotionLineNumner);
            return;
    }
    // Assume a velocity of zero at the first move.
    nextChange = vec[currentLine];
    lastPosition.x = vec[currentLine + 1];
    lastPosition.y = vec[currentLine + 2];
    lastPosition.z = is3D ? vec[currentLine + 3] : 0;

    targetPosition.x = lastPosition.x;
    targetPosition.y = lastPosition.y;
    targetPosition.z = lastPosition.z;

    currentLine += (is3D ? 4 : 3);
}

void BonnMotionMobilityClient::setTargetPosition()
{
    const BonnMotionFile::Line& vec = *lines;
    if (currentLine + (is3D ? 3 : 2) >= (int)vec.size()) {
        nextChange = -1;
        stationary = true;
        targetPosition = lastPosition;
        emit(BonnMotionMobilityServer::bonnMotionTargetReached, (double)bonnMotionLineNumner);
        return;
    }
    nextChange = vec[currentLine];
    targetPosition.x = vec[currentLine + 1];
    targetPosition.y = vec[currentLine + 2];
    targetPosition.z = is3D ? vec[currentLine + 3] : 0;
    currentLine += (is3D ? 4 : 3);
}

} /* namespace crownet */
