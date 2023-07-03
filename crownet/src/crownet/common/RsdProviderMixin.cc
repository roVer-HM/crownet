/*
 * RsdProviderMixin.cc
 *
 *  Created on: Jun 23, 2023
 *      Author: vm-sts
 */

#include "RsdProviderMixin.h"

using namespace inet;


namespace crownet {

bool operator==(const int& a, const RsdIdPair& b){
    return b == a;
}

bool operator==(const RsdIdPair& a, const int& b){
    return a.current.id == b;
}


void RsdListener::receiveSignal(cComponent *source, simsignal_t signalID, intval_t i, cObject *details){
    if (signalID == servingCell_){
        rsd.previous = rsd.current;
        rsd.current.id = i;
        rsd.current.time = simTime();
    }
}



} /* namespace crownet */
