/*
 * Beacon.h
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#pragma once

#include "inet/common/INETDefs.h"
#include "inet/common/geometry/common/Coord.h"
#include <omnetpp/simtime_t.h>


namespace crownet {

class NeighborhoodTableEntry;

std::ostream&  operator<<(std::ostream& os, const NeighborhoodTableEntry &beacon);

class NeighborhoodTableEntry : public omnetpp::cObject {
public:
    virtual ~NeighborhoodTableEntry() = default;
    NeighborhoodTableEntry(){};
    NeighborhoodTableEntry(int nodeId, omnetpp::simtime_t timeSend, omnetpp::simtime_t timeReceived, inet::Coord pos, inet::Coord epsilon)
        : nodeId(nodeId), timeSend(timeSend), timeReceived(timeReceived), pos(pos), epsilon(epsilon){}
    NeighborhoodTableEntry(int nodeId, omnetpp::simtime_t timeSend,  inet::Coord pos, inet::Coord epsilon)
        : nodeId(nodeId), timeSend(timeSend), timeReceived(omnetpp::simTime()), pos(pos), epsilon(epsilon){}
    NeighborhoodTableEntry(int nodeId, omnetpp::simtime_t timeSend, omnetpp::simtime_t timeReceived, inet::Coord pos)
            : nodeId(nodeId), timeSend(timeSend), timeReceived(timeReceived), pos(pos), epsilon(){}
    NeighborhoodTableEntry(int nodeId, omnetpp::simtime_t timeSend, inet::Coord pos)
            : nodeId(nodeId), timeSend(timeSend), timeReceived(omnetpp::simTime()), pos(pos), epsilon(){}

    NeighborhoodTableEntry(const NeighborhoodTableEntry& other) { copy(other); }

    NeighborhoodTableEntry& operator=(const NeighborhoodTableEntry& other);

    int getNodeId() const { return nodeId; }
    omnetpp::simtime_t getTimeSend() const {return timeSend;}
    omnetpp::simtime_t getTimeReceived() const {return timeReceived;}
    inet::Coord& getPos() { return pos;}
    inet::Coord getPos() const { return pos;}
    inet::Coord& getEpsilon() {return epsilon;}
    inet::Coord getEpsilon() const {return epsilon;}

    void setNodeId(const int& nodeId) {this->nodeId = nodeId;}
    void setTimeSend( const  omnetpp::simtime_t& time) {this->timeSend=time;}
    void setTimeReceived( const  omnetpp::simtime_t& time) {this->timeReceived=time;}
    void setPos( const inet::Coord& pos) {this->pos = pos;}
    void setEpsilon( const inet::Coord& epsilon) {this->epsilon=epsilon;}
    virtual std::string str() const override;

private:
    void copy(const NeighborhoodTableEntry& other);

private:
    int nodeId;
    omnetpp::simtime_t timeSend;
    omnetpp::simtime_t timeReceived;
    inet::Coord pos;
    inet::Coord epsilon;
};

} /* namespace crownet */

