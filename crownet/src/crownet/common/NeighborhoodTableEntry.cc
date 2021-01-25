/*
 * NeighborhoodTableEntry.cc
 *
 *  Created on: Jan 23, 2021
 *      Author: sts
 */

#include "NeighborhoodTableEntry.h"

namespace crownet {

std::ostream&  operator<<(std::ostream& os, const NeighborhoodTableEntry &beacon){
    os << "{id: " << beacon.getNodeId()
            << " tSend: " << beacon.getTimeSend()
            << " tReceived: " << beacon.getTimeReceived()
            << " pos: " << beacon.getPos()
            << " epsilon: " << beacon.getEpsilon();
    return os;
}


NeighborhoodTableEntry& NeighborhoodTableEntry::operator=(const NeighborhoodTableEntry& other) {
    if (this == &other)
        return *this;
    copy(other);
    return *this;
}


void NeighborhoodTableEntry::copy(const NeighborhoodTableEntry& other) {
    nodeId=other.nodeId;
    timeSend=other.timeSend;
    timeReceived=other.timeReceived;
    pos=other.pos;
    epsilon=other.epsilon;
}

std::string NeighborhoodTableEntry::str() const {
    std::stringstream s;
    s << *this;
    return s.str();
}



} /* namespace crownet */
