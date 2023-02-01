//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#include "crownet/neighbourhood/EntropyNeigborhoodTableClient.h"

#include "inet/common/ModuleAccess.h"

using namespace omnetpp;
using namespace inet;



namespace crownet {

Define_Module(EntropyNeigborhoodTableClient);


void EntropyNeigborhoodTableClient::initialize(int stage) {
    MobilityProviderMixin<cSimpleModule>::initialize(stage);
    if (stage == INITSTAGE_LOCAL){
        globalTable = getModuleFromPar<GlobalEntropyMap>(par("globalTable"), this);
        converter = inet::getModuleFromPar<OsgCoordConverterProvider>(
                        par("coordConverterModule"), this)
                        ->getConverter();
        dist = par("distance");
    }
}

void EntropyNeigborhoodTableClient::handleMessage(cMessage *msg) {
    throw cRuntimeError("EntropyNeigborhoodTableClient does not hanlde messages");
}



const int EntropyNeigborhoodTableClient::getSize(){
    int count = 0;
    for (const auto& t : iter()){
        count++;
    }
    return count;
}

const NeighborhoodTableValue_t EntropyNeigborhoodTableClient::updateGetGlobalValue(const inet::Coord& pos){
    return globalTable->getValue(pos);
}

const bool EntropyNeigborhoodTableClient::currentCellOnly() const{
    return dist <= 0.0;
}

// iterator default to all elements in map
NeighborhoodTableIter_t
EntropyNeigborhoodTableClient::iter() {
    NeighborhoodTablePred_t pred;
    if (dist > 0.0){
        pred = INeighborhoodTable::inRadius_pred(getPosition(), dist);
        return globalTable->iter(pred);
    }
    return globalTable->iter();
}

NeighborhoodTableIter_t
EntropyNeigborhoodTableClient::iter(NeighborhoodTablePred_t predicate){
    return globalTable->iter(predicate);
}


}
