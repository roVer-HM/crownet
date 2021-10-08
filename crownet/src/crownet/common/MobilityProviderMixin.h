/*
 * MobilityProviderMixin.h
 *
 *  Created on: Oct 5, 2021
 *      Author: vm-sts
 */

#pragma once

#include "inet/mobility/contract/IMobility.h"
#include "inet/common/geometry/common/Coord.h"
#include "inet/common/InitStages.h"
#include "inet/common/ModuleAccess.h"
#include <omnetpp/checkandcast.h>

using namespace inet;
namespace crownet {

template<typename T>
class MobilityProviderMixin : public T {
public:
    virtual void initialize(int stage) override;

    const inet::Coord getPosition() {return mobility->getCurrentPosition();}
    IMobility* getMobility() { return mobility;}

    template <typename M>
    M* castMobility();

private:
    IMobility* mobility;

};

template<typename T>
template<typename M>
inline M* MobilityProviderMixin<T>::castMobility(){
    return omnetpp::check_and_cast<M*>(mobility);
}


template<typename T>
inline void MobilityProviderMixin<T>::initialize(int stage){
    T::initialize(stage);
    if(stage == INITSTAGE_LOCAL){
        if (T::hasPar("mobilityModule")){
            mobility = getModuleFromPar<IMobility>(T::par("mobilityModule"), this);
        } else {
            throw cRuntimeError("MobilityProviderMixin must provide the mobilityModule mobilityModule");
        }
    }
}



};



