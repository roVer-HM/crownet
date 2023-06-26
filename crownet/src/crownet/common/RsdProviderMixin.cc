/*
 * RsdProviderMixin.cc
 *
 *  Created on: Jun 23, 2023
 *      Author: vm-sts
 */

#include "RsdProviderMixin.h"

using namespace inet;


namespace crownet {


void RsdListener::receiveSignal(cComponent *source, simsignal_t signalID, intval_t i, cObject *details){
    if (signalID == servingCell_){
        resourceSharingDomainId_prev = resourceSharingDomainId;
        resourceSharingDomainId = i;
    }
}



} /* namespace crownet */
